// Copyright (C) 2003 David Griffiths <dave@pawfal.org>
// adapted to FreeJ by jaromil
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <config.h>
#ifdef WITH_AUDIO

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#include <audio_pipewire.h>

#include <jutils.h>
#include <algorithm>
#include <cerrno>
#include <thread>
#include <pipewire/pipewire.h>
#include <pipewire/filter.h>
#include <pipewire/impl.h>
#include <spa/utils/string.h>

PipeWireClient *PipeWireClient::m_Singleton = nullptr;
std::atomic<bool> PipeWireClient::m_Attached{false};
std::atomic<unsigned long> PipeWireClient::m_BufferSize{0}, PipeWireClient::m_SampleRate{0};
void (*PipeWireClient::RunCallback)(void *, unsigned int) = nullptr;
void *PipeWireClient::RunContext = nullptr;
std::map<int, PipeWireClient::AudioPort *> PipeWireClient::m_InputPortMap;
std::map<int, PipeWireClient::AudioPort *> PipeWireClient::m_OutputPortMap;

#ifdef FREEJ_TEST_AUDIO_GRAPH
#define FREEJ_TEST_AUDIO_GRAPH_IMPL
#include "audio-graph-fixture.h"
#else
struct PipeWireClient::Graph {
  struct Lock {
    explicit Lock(pw_thread_loop *loop) : loop(loop) { pw_thread_loop_lock(loop); }
    ~Lock() { pw_thread_loop_unlock(loop); }
    pw_thread_loop *loop;
  };

  struct Port {
    uint32_t node;
    std::string name;
    bool input;
  };
  struct Link {
    uint32_t input, output;
  };

  explicit Graph(PipeWireClient &client) : client(client) {}

  ~Graph() {
    if (running) pw_thread_loop_stop(loop);
    for (const auto &route : routes) pw_proxy_destroy(route.second);
    if (filter) pw_filter_destroy(filter);
    if (registry) pw_proxy_destroy(reinterpret_cast<pw_proxy *>(registry));
    if (core) pw_core_disconnect(core);
    if (context) pw_context_destroy(context);
    if (loop) pw_thread_loop_destroy(loop);
  }

  bool open(const std::string &client_name) {
    static std::once_flag initialized;
    std::call_once(initialized, [] { pw_init(nullptr, nullptr); });

    loop = pw_thread_loop_new("freej-pipewire", nullptr);
    if (!loop || pw_thread_loop_start(loop) < 0) return false;
    running = true;
    Lock lock(loop);

    pw_properties *properties = nullptr;
#ifdef FREEJ_STRICT_STATIC
    properties = pw_properties_new(PW_KEY_CONFIG_NAME, "null",
                                   "support.dbus", "false",
                                   "context.modules.allow-empty", "true", nullptr);
    if (!properties) return false;
#endif
    context = pw_context_new(pw_thread_loop_get_loop(loop), properties, 0);
    if (!context) return false;
#ifdef FREEJ_STRICT_STATIC
    if (!pw_context_load_module(context, "libpipewire-module-protocol-native", nullptr, nullptr) ||
        !pw_context_load_module(context, "libpipewire-module-client-node", nullptr, nullptr))
      return false;
#endif

    core = pw_context_connect(context, nullptr, 0);
    if (!core) return false;
    static const pw_core_events core_events = [] {
      pw_core_events events{};
      events.version = PW_VERSION_CORE_EVENTS;
      events.info = &Graph::core_info;
      events.done = &Graph::done;
      events.error = &Graph::core_error;
      return events;
    }();
    pw_core_add_listener(core, &core_hook, &core_events, this);

    registry = pw_core_get_registry(core, PW_VERSION_REGISTRY, 0);
    if (!registry) return false;
    static const pw_registry_events registry_events = {
      PW_VERSION_REGISTRY_EVENTS, &Graph::global, &Graph::global_remove
    };
    pw_registry_add_listener(registry, &registry_hook, &registry_events, this);
    properties = pw_properties_new(PW_KEY_NODE_NAME, client_name.c_str(),
                                   PW_KEY_NODE_ALWAYS_PROCESS, "true",
                                   PW_KEY_MEDIA_TYPE, "Audio",
                                   PW_KEY_MEDIA_CATEGORY, "Filter", nullptr);
    if (!properties) return false;
    filter = pw_filter_new(core, client_name.c_str(), properties);
    if (!filter) return false;
    static const pw_filter_events filter_events = [] {
      pw_filter_events events{};
      events.version = PW_VERSION_FILTER_EVENTS;
      events.state_changed = &Graph::state_changed;
      events.process = &Graph::process;
      return events;
    }();
    pw_filter_add_listener(filter, &filter_hook, &filter_events, this);
    m_SampleRate = 48000;
    m_BufferSize = 1024;
    if (pw_filter_connect(filter, PW_FILTER_FLAG_RT_PROCESS, nullptr, 0) < 0 || !sync())
      return false;
    timespec deadline{};
    if (pw_thread_loop_get_time(loop, &deadline, 2000000000) < 0) return false;
    while (!failed && state == PW_FILTER_STATE_CONNECTING)
      if (pw_thread_loop_timed_wait_full(loop, &deadline) < 0) return false;
    if (failed || state < PW_FILTER_STATE_PAUSED) return false;
    m_Attached = true;
    return true;
  }

  bool sync() {
    if (failed) return false;
    const int seq = pw_core_sync(core, PW_ID_CORE, 0);
    if (seq < 0) return false;
    timespec deadline{};
    if (pw_thread_loop_get_time(loop, &deadline, 2000000000) < 0) return false;
    while (!failed && completed != seq)
      if (pw_thread_loop_timed_wait_full(loop, &deadline) < 0) return false;
    return !failed;
  }

  void *add_port(AudioPort &port, pw_direction direction) {
    std::lock_guard<std::mutex> operation(operations);
    Lock lock(loop);
    if (failed) return nullptr;
    auto *properties = pw_properties_new(PW_KEY_PORT_NAME, port.Name.c_str(),
                                         PW_KEY_FORMAT_DSP, "32 bit float mono audio", nullptr);
    if (!properties) return nullptr;
    auto *handle = pw_filter_add_port(filter, direction, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                                      1, properties, nullptr, 0);
    if (handle && direction == PW_DIRECTION_INPUT) local_inputs.emplace(port.Name, &port);
    return handle;
  }

  void remove_port(AudioPort &port) {
    std::lock_guard<std::mutex> operation(operations);
    Lock lock(loop);
    local_inputs.erase(port.Name);
    disconnect(port);
    pw_filter_remove_port(port.Port);
  }

  std::string name(const Port &port) const {
    const auto node = nodes.find(port.node);
    return node == nodes.end() ? "" : node->second + ":" + port.name;
  }

  void connect(AudioPort &port, bool input, const std::string &peer) {
    std::lock_guard<std::mutex> operation(operations);
    Lock lock(loop);
    if (failed) return;
    disconnect(port);
    if (peer.empty() || !sync()) return;

    auto local = SPA_ID_INVALID, remote = SPA_ID_INVALID;
    const auto node_id = pw_filter_get_node_id(filter);
    for (const auto &item : ports) {
      const auto &candidate = item.second;
      if (candidate.input == input && candidate.node == node_id && candidate.name == port.Name)
        local = item.first;
      else if (candidate.input != input && name(candidate) == peer)
        remote = item.first;
    }
    if (local == SPA_ID_INVALID || remote == SPA_ID_INVALID) {
      error("Could not resolve PipeWire port %s for %s", peer.c_str(), port.Name.c_str());
      return;
    }

    const auto output = std::to_string(input ? remote : local);
    const auto destination = std::to_string(input ? local : remote);
    auto *properties = pw_properties_new(PW_KEY_LINK_OUTPUT_PORT, output.c_str(),
                                         PW_KEY_LINK_INPUT_PORT, destination.c_str(), nullptr);
    if (!properties) return;
    auto *link = static_cast<pw_proxy *>(pw_core_create_object(
        core, "link-factory", PW_TYPE_INTERFACE_Link, PW_VERSION_LINK, &properties->dict, 0));
    pw_properties_free(properties);
    if (link)
      routes.emplace(&port, link);
    else
      error("Could not link PipeWire port %s to %s", port.Name.c_str(), peer.c_str());
  }

  static float *buffer(void *port, unsigned frames) {
    return static_cast<float *>(pw_filter_get_dsp_buffer(port, frames));
  }

  void names(std::vector<std::string> &inputs, std::vector<std::string> &outputs) {
    std::lock_guard<std::mutex> operation(operations);
    Lock lock(loop);
    if (!sync()) return;
    for (const auto &item : ports) {
      const auto label = name(item.second);
      if (!label.empty())
        (item.second.input ? inputs : outputs).push_back(label);
    }
  }

private:
  std::mutex operations;
  pw_thread_loop *loop = nullptr;
  std::map<uint32_t, Port> ports;

  void disconnect(AudioPort &port) {
    auto route = routes.find(&port);
    if (route == routes.end()) return;
    pw_proxy_destroy(route->second);
    routes.erase(route);
  }

  void refresh_links() {
    const auto node_id = pw_filter_get_node_id(filter);
    for (const auto &local : local_inputs) {
      const bool linked = std::any_of(links.begin(), links.end(), [&](const std::pair<const uint32_t, Link> &entry) {
        const auto port = ports.find(entry.second.input);
        return port != ports.end() && port->second.node == node_id && port->second.name == local.first;
      });
      local.second->linked.store(linked, std::memory_order_relaxed);
    }
  }

  static void global(void *data, uint32_t id, uint32_t, const char *type, uint32_t,
                     const spa_dict *properties) {
    if (!properties) return;
    auto &graph = *static_cast<Graph *>(data);
    if (strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
      if (const auto *name = spa_dict_lookup(properties, PW_KEY_NODE_NAME))
        graph.nodes[id] = name;
    } else if (strcmp(type, PW_TYPE_INTERFACE_Port) == 0) {
      const auto *name = spa_dict_lookup(properties, PW_KEY_PORT_NAME);
      const auto *direction = spa_dict_lookup(properties, PW_KEY_PORT_DIRECTION);
      uint32_t node;
      if (name && direction && spa_atou32(spa_dict_lookup(properties, PW_KEY_NODE_ID), &node, 10))
        graph.ports.emplace(id, Port{node, name, strcmp(direction, "in") == 0});
    } else if (strcmp(type, PW_TYPE_INTERFACE_Link) == 0) {
      Link link;
      if (spa_atou32(spa_dict_lookup(properties, PW_KEY_LINK_INPUT_PORT), &link.input, 10) &&
          spa_atou32(spa_dict_lookup(properties, PW_KEY_LINK_OUTPUT_PORT), &link.output, 10)) {
        graph.links.emplace(id, link);
        if (graph.filter) graph.refresh_links();
      }
    }
  }

  static void global_remove(void *data, uint32_t id) {
    auto &graph = *static_cast<Graph *>(data);
    graph.nodes.erase(id);
    const bool removed_port = graph.ports.erase(id);
    const bool removed_link = graph.links.erase(id);
    if (graph.filter && (removed_port || removed_link)) graph.refresh_links();
  }

  static void core_info(void *, const pw_core_info *info) {
    if (!info->props) return;
    uint32_t rate;
    if (spa_atou32(spa_dict_lookup(info->props, "default.clock.rate"), &rate, 10) && rate)
      m_SampleRate = rate;
  }

  static void done(void *data, uint32_t id, int seq) {
    auto &graph = *static_cast<Graph *>(data);
    if (id == PW_ID_CORE) {
      graph.completed = seq;
      pw_thread_loop_signal(graph.loop, false);
    }
  }

  void fail() {
    failed = true;
    if (m_Attached.exchange(false)) client.shutdown_pending_ = true;
    pw_thread_loop_signal(loop, false);
  }

  static void core_error(void *data, uint32_t id, int, int result, const char *message) {
    auto &graph = *static_cast<Graph *>(data);
    error("PipeWire: %s", message);
    if (id == PW_ID_CORE && (result == -EPIPE || result == -ECONNRESET))
      graph.fail();
  }

  static void state_changed(void *data, pw_filter_state, pw_filter_state state, const char *) {
    auto &graph = *static_cast<Graph *>(data);
    graph.state = state;
    if (state == PW_FILTER_STATE_ERROR || state == PW_FILTER_STATE_UNCONNECTED)
      graph.fail();
    pw_thread_loop_signal(graph.loop, false);
  }

  static void process(void *data, spa_io_position *position) {
    auto &graph = *static_cast<Graph *>(data);
    Process(&graph.client, position);
  }

  PipeWireClient &client;
  pw_context *context = nullptr;
  pw_core *core = nullptr;
  pw_registry *registry = nullptr;
  pw_filter *filter = nullptr;
  spa_hook core_hook{}, registry_hook{}, filter_hook{};
  int completed = -1;
  pw_filter_state state = PW_FILTER_STATE_UNCONNECTED;
  bool running = false, failed = false;
  std::map<uint32_t, std::string> nodes;
  std::map<uint32_t, Link> links;
  std::map<std::string, AudioPort *> local_inputs;
  std::map<AudioPort *, pw_proxy *> routes;
};
#endif

///////////////////////////////////////////////////////

PipeWireClient::PipeWireClient() :
audio_mix_ring(nullptr),
first(nullptr),
m_ringbufferchannels(0),
m_ringbuffer(nullptr),
m_NextInputID(0),
m_Encoded(false)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////

PipeWireClient::~PipeWireClient()
{
	Detach();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PipeWireClient::isEncoded(bool enc)
{
  m_Encoded = enc;
}

std::shared_ptr<RecordingRings> PipeWireClient::BeginRecording()
{
  std::lock_guard<std::mutex> lock(output_mutex_);
  if (!m_Attached || !recording_ || recording_->rate.load()) return {};
  // A previous encoder can leave a tail. Only this newly admitted reader
  // advances the read cursor, independently of the device writer.
  for (auto *ring : {first, audio_mix_ring})
    ringbuffer_read_advance(ring, ringbuffer_read_space(ring));
  recording_->channels.store(std::max(1, m_ringbufferchannels.load()));
  recording_->rate.store(m_SampleRate.load());
  return recording_;
}

void PipeWireClient::SetCallback(void(*run)(void*, unsigned int), void *context)
{
  std::lock_guard<std::recursive_mutex> lock(callback_mutex_);
  RunCallback = run;
  RunContext = context;
}

void PipeWireClient::ClearCallback(void *context)
{
  std::lock_guard<std::recursive_mutex> lock(callback_mutex_);
  if (RunContext == context) {
    RunCallback = nullptr;
    RunContext = nullptr;
  }
}

void PipeWireClient::dispatch_shutdown()
{
  if (!shutdown_pending_.exchange(false)) return;
  std::lock_guard<std::recursive_mutex> lock(callback_mutex_);
  if (RunCallback) RunCallback(RunContext, 0);
}

bool PipeWireClient::IsAttached()
{
  dispatch_shutdown();
  return m_Attached.load();
}

bool PipeWireClient::Attach(const std::string &ClientName)
{
  dispatch_shutdown();
	std::lock_guard<std::mutex> inputs(input_mutex_);
	std::lock_guard<std::mutex> lock(output_mutex_);
	if (m_Attached) return true;
	// A server shutdown can leave an inactive client to retire before reopening.
	if (graph_)
	  detach_locked();

  recording_ = std::make_shared<RecordingRings>();
  audio_mix_ring = recording_->mix.get();
  first = recording_->input.get();
  graph_ = std::unique_ptr<Graph>(new Graph(*this));
  if (!audio_mix_ring || !first || !graph_->open(ClientName)) {
    error("Could not connect to the native PipeWire graph");
    detach_locked();
    return false;
  }

  ++session_;
  return m_Attached.load();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PipeWireClient::Detach()
{
  dispatch_shutdown();
	std::lock_guard<std::mutex> inputs(input_mutex_);
	std::lock_guard<std::mutex> lock(output_mutex_);
	detach_locked();
}

void PipeWireClient::detach_locked()
{
  m_Attached = false;
  graph_.reset();
  m_SampleRate = m_BufferSize = 0;
	for (auto &entry : m_InputPortMap) {
	  if (entry.second->capture) entry.second->capture->gap();
	  delete entry.second;
	}
	m_InputPortMap.clear();
	input_lists_[0].count = input_lists_[1].count = 0;
	recording_.reset();
	m_Encoded = false;
	audio_mix_ring = first = nullptr;
	m_ringbuffer = nullptr;
	m_ringbufferchannels = 0;
	output_buffer_.clear();
	for (auto &port : m_OutputPortMap)
	  delete port.second;
	m_OutputPortMap.clear();
	// PipeWire shutdown has stopped callback execution before retiring the published ports.
	output_port_count_.store(0, std::memory_order_relaxed);

	// tells ssm to go back to non callback mode
	//if (RunCallback) RunCallback(RunContext, false);
}

/////////////////////////////////////////////////////////////////////////////////////////////

namespace {
void deinterleave(const void *input, float *output, int channels, int channel,
                  unsigned count)
{
  auto *samples = static_cast<const float *>(input);
  for (unsigned i = 0; i < count; ++i)
    output[i] = samples[i * channels + channel];
}
}

void PipeWireClient::Process(void *self, spa_io_position *position)
{
  if (!position || !position->clock.duration || !position->clock.rate.num)
    return;
  if (position->clock.duration > UINT32_MAX)
    return;
  const unsigned nframes = position->clock.duration;
  const unsigned rate = position->clock.rate.denom / position->clock.rate.num;
  const auto frame = static_cast<uint32_t>(position->clock.position);
  m_SampleRate.store(rate, std::memory_order_relaxed);
  m_BufferSize.store(nframes, std::memory_order_relaxed);
  auto &client = *static_cast<PipeWireClient *>(self);
  // The real-time reader publishes the interval in which it may access
  // input handles so control-side removal can wait for that reader only.
  client.input_epoch_.fetch_add(1);
  const auto &inputs = client.input_lists_[client.active_inputs_.load()];
  const unsigned recording_rate = client.recording_->rate.load(std::memory_order_acquire);
  const bool recording = recording_rate ? recording_rate == rate : client.m_Encoded.load(std::memory_order_relaxed);
  bool first_connected = true;
  for (size_t i = 0; i < inputs.count; ++i) {
    auto &port = *inputs.ports[i];
    const auto *in = Graph::buffer(port.Port, nframes);
    if (!port.linked.load(std::memory_order_relaxed))
      in = nullptr;
    if (!in) {
      if (port.capture) port.capture->gap();
      continue;
    }
    if (port.capture) port.capture->write(in, nframes, frame, rate);
    if (first_connected && recording) {
      const size_t bytes = nframes * sizeof(float);
      if (ringbuffer_write_space(client.first) >= bytes)
        ringbuffer_write(client.first, reinterpret_cast<const char *>(in), bytes);
    }
    first_connected = false;
  }
  client.input_epoch_.fetch_add(1);

	{
	  // A control thread may change the route. The callback never waits for it.
	  std::unique_lock<std::mutex> lock(client.output_mutex_, std::try_to_lock);
	  const int channels = lock ? client.m_ringbufferchannels.load(std::memory_order_relaxed) : 0;
	  const std::size_t frame_bytes = channels * sizeof(float);
	  const std::size_t period_bytes = nframes * frame_bytes;
	  bool output_available = false;
	  if (frame_bytes && client.m_ringbuffer && client.output_rate_ == rate && period_bytes <= client.output_buffer_.size()) {
	    auto *samples = client.output_buffer_.data();
	    const auto bytes = std::min(period_bytes, ringbuffer_read_space(client.m_ringbuffer));
	    const auto received = ringbuffer_read(client.m_ringbuffer, samples, bytes - bytes % frame_bytes);
	    if (received) {
	      // The encoder receives complete source samples, including the tail
	      // of a clip. Playback alone gets zero filling to the PipeWire quantum.
	      if (recording && (!recording_rate || client.recording_->channels.load() == static_cast<unsigned>(channels))
	          && ringbuffer_write_space(client.audio_mix_ring) >= received)
	        ringbuffer_write(client.audio_mix_ring, samples, received);
	      memset(samples + received, 0, period_bytes - received);
	      output_available = true;
	    }
	  }

	  // Port slots are append-only while PipeWire can call Process. An acquired
	  // count makes their handles visible even when the route lock is held.
	  const int ports = client.output_port_count_.load(std::memory_order_acquire);
	  for (int channel = 0; channel < ports; ++channel) {
	    auto *output = Graph::buffer(client.output_ports_[channel], nframes);
	    if (!output)
	      continue;
	    if (output_available && channel < channels)
	      deinterleave(client.output_buffer_.data(), output, channels, channel, nframes);
	    else
	      memset(output, 0, nframes * sizeof(*output));
	  }
	}
}

int PipeWireClient::SetRingbufferPtr(ringbuffer_t *rb, int rate, int channels) {
	if (!rb || channels <= 0 || channels > MAX_OUTPUTPORTS)
	  return EINVAL;
	std::lock_guard<std::mutex> lock(output_mutex_);
	if (!m_Attached)
	  return EINVAL;
	while (output_port_count_.load(std::memory_order_relaxed) < channels) {
	  if (add_output_port_locked() < 0)
	    return ENODEV;
	}
	const std::size_t period = std::max(8192ul, m_BufferSize.load());
	output_rate_ = rate;
	output_buffer_.resize(period * channels * sizeof(float));
	ringbuffer_reset(rb);
	m_ringbufferchannels = channels;
	m_ringbuffer = rb;
	return 0;
}

void PipeWireClient::ResetRingbuffer(ringbuffer_t *rb, bool disconnect, unsigned rate) {
	if (auto *client = m_Singleton) {
	  std::lock_guard<std::mutex> lock(client->output_mutex_);
	  if (client->m_ringbuffer == rb) {
	    ringbuffer_reset(rb);
	    if (rate) client->output_rate_ = rate;
	    if (disconnect) {
	      client->m_ringbuffer = nullptr;
	      // Preserve the recording format until a different supplier binds.
	    }
	  }
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////

int PipeWireClient::AddInputPort(std::shared_ptr<AudioCapture> capture)
{
  std::lock_guard<std::mutex> lock(input_mutex_);
  if (!m_Attached || m_InputPortMap.size() == MAX_INPUTPORTS) return -1;
  std::unique_ptr<AudioPort> port(new AudioPort);
  const int id = m_NextInputID++;
  port->Name = "In" + std::to_string(id);
  port->Port = graph_->add_port(*port, PW_DIRECTION_INPUT);
  if (!port->Port) return -1;
  port->capture = std::move(capture);
  m_InputPortMap[id] = port.release();
  publish_inputs_locked();
  return id;
}

void PipeWireClient::publish_inputs_locked()
{
  const unsigned next = 1 - active_inputs_.load();
  auto &list = input_lists_[next];
  list.count = 0;
  for (const auto &entry : m_InputPortMap) list.ports[list.count++] = entry.second;
  active_inputs_.store(next);

  // Readers starting after the swap cannot acquire the old list. Only
  // wait for the reader already in flight, never for the next PipeWire quantum.
  const auto epoch = input_epoch_.load();
  if (epoch & 1u) {
    while (input_epoch_.load() == epoch) std::this_thread::yield();
  }
}

void PipeWireClient::RemoveInputPort(int id)
{
  std::lock_guard<std::mutex> lock(input_mutex_);
  auto it = m_InputPortMap.find(id);
  if (it == m_InputPortMap.end()) return;
  std::unique_ptr<AudioPort> port(it->second);
  m_InputPortMap.erase(it);
  publish_inputs_locked();
  if (graph_) graph_->remove_port(*port);
}

/////////////////////////////////////////////////////////////////////////////////////////////

int PipeWireClient::AddOutputPort()
{
	std::lock_guard<std::mutex> lock(output_mutex_);
	return add_output_port_locked();
}

int PipeWireClient::add_output_port_locked()
{
	static_assert(ATOMIC_INT_LOCK_FREE == 2, "PipeWire output publication must be lock-free");
	const int channel = output_port_count_.load(std::memory_order_relaxed);
	if (!m_Attached || channel == MAX_OUTPUTPORTS)
	  return -1;

	char name[32];
	snprintf(name, sizeof(name), "Out%d", channel);
	auto *port = new AudioPort;
	port->Name = name;
	port->Port = graph_->add_port(*port, PW_DIRECTION_OUTPUT);
	if (!port->Port) {
	  delete port;
	  return -1;
	}
	m_OutputPortMap[channel] = port;
	output_ports_[channel] = port->Port;
	output_port_count_.store(channel + 1, std::memory_order_release);
	return channel;
}


/////////////////////////////////////////////////////////////////////////////////////////////

void PipeWireClient::GetPortNames(std::vector<std::string> &inputs,
                                  std::vector<std::string> &outputs)
{
  std::lock_guard<std::mutex> lock(output_mutex_);
  inputs.clear();
  outputs.clear();
  if (!m_Attached) return;
  graph_->names(inputs, outputs);
}

void PipeWireClient::ConnectInput(int id, const std::string &source)
{
  std::lock_guard<std::mutex> lock(input_mutex_);
  const auto port = m_InputPortMap.find(id);
  if (m_Attached && port != m_InputPortMap.end())
    graph_->connect(*port->second, true, source);
}

void PipeWireClient::ConnectOutput(int id, const std::string &sink)
{
  std::lock_guard<std::mutex> lock(output_mutex_);
  const auto port = m_OutputPortMap.find(id);
  if (m_Attached && port != m_OutputPortMap.end())
    graph_->connect(*port->second, false, sink);
}

void PipeWireClient::DisconnectInput(int id)
{
  ConnectInput(id, "");
}

void PipeWireClient::DisconnectOutput(int id)
{
  ConnectOutput(id, "");
}

void PipeWireClient::SetInputBuf(int ID, float* s)
{
  std::lock_guard<std::mutex> lock(input_mutex_);
	if(m_InputPortMap.find(ID)!=m_InputPortMap.end()) m_InputPortMap[ID]->Buf=s;
	else std::cerr<<"Could not find port ID "<<ID<<std::endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PipeWireClient::SetOutputBuf(int ID, float* s)
{
  std::lock_guard<std::mutex> lock(output_mutex_);
	if(m_OutputPortMap.find(ID)!=m_OutputPortMap.end()) m_OutputPortMap[ID]->Buf=s;
	else error("Could not find port ID %u", ID);
}


#endif
