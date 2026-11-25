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

#include <audio_jack.h>

#include <jutils.h>
#include <algorithm>
#include <cerrno>
#include <signal.h>
#include <thread>

namespace {
// JACK1 can write to a dead server socket during close or unregister. Keep
// its SIGPIPE local to this control call even in an embedding using SIG_DFL.
template<typename Request>
int server_request(Request request)
{
  sigset_t pipe, previous, pending;
  sigemptyset(&pipe);
  sigaddset(&pipe, SIGPIPE);
  const bool blocked = pthread_sigmask(SIG_BLOCK, &pipe, &previous) == 0;
  const bool consume = blocked && !sigismember(&previous, SIGPIPE) &&
                       !sigpending(&pending) && !sigismember(&pending, SIGPIPE);
  const int result = request();
  if (consume && !sigpending(&pending) && sigismember(&pending, SIGPIPE)) {
    int signal;
    sigwait(&pipe, &signal);
  }
  if (blocked) pthread_sigmask(SIG_SETMASK, &previous, nullptr);
  return result;
}
}

JackClient*       JackClient::m_Singleton  = NULL;
std::atomic<bool> JackClient::m_Attached{false};
std::atomic<unsigned long> JackClient::m_BufferSize{0}, JackClient::m_SampleRate{0};
void            (*JackClient::RunCallback)(void*, unsigned int BufSize)=NULL;
void             *JackClient::RunContext   = NULL;	
jack_client_t    *JackClient::m_Client     = NULL;
std::map<int,JackClient::JackPort*> JackClient::m_InputPortMap;
std::map<int,JackClient::JackPort*> JackClient::m_OutputPortMap;

///////////////////////////////////////////////////////

JackClient::JackClient() :
audio_mix_ring(nullptr),
first(nullptr),
m_ringbufferchannels(0),
m_ringbuffer(nullptr),
m_NextInputID(0),
m_Encoded(false)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////

JackClient::~JackClient()	
{
	Detach();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void JackClient::isEncoded(bool enc)
{
  m_Encoded = enc;
}

std::shared_ptr<RecordingRings> JackClient::BeginRecording()
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

void JackClient::SetCallback(void(*run)(void*, unsigned int), void *context)
{
  std::lock_guard<std::recursive_mutex> lock(callback_mutex_);
  RunCallback = run;
  RunContext = context;
}

void JackClient::ClearCallback(void *context)
{
  std::lock_guard<std::recursive_mutex> lock(callback_mutex_);
  if (RunContext == context) {
    RunCallback = nullptr;
    RunContext = nullptr;
  }
}

void JackClient::dispatch_shutdown()
{
  if (!shutdown_pending_.exchange(false)) return;
  std::lock_guard<std::recursive_mutex> lock(callback_mutex_);
  if (RunCallback) RunCallback(RunContext, 0);
}

bool JackClient::IsAttached()
{
  dispatch_shutdown();
  return m_Attached.load();
}

bool JackClient::Attach(const std::string &ClientName)
{
  dispatch_shutdown();
	std::lock_guard<std::mutex> inputs(input_mutex_);
	std::lock_guard<std::mutex> lock(output_mutex_);
	if (m_Attached) return true;
	// A server shutdown can leave an inactive client to retire before reopening.
	if (m_Client)
	  detach_locked();

/* jack_client_new is now deprecated (Oct 2010)
   m_Client = jack_client_new(ClientName.c_str());
   
   if (!m_Client)
   {
   error("jack server not running? client_new returns %p", m_Client);
   return false;
   } */
        m_Client = 
            jack_client_open(ClientName.c_str(),
                             (jack_options_t) (JackNullOption | JackNoStartServer), NULL);
        if (!m_Client) {
            error("jack server not running?", m_Client);
            return false;
        }

	jack_set_process_callback(m_Client, JackClient::Process, this);
	jack_set_sample_rate_callback (m_Client, JackClient::OnSRateChange, 0);
	jack_on_shutdown (m_Client, JackClient::OnJackShutdown, this);

	m_InputPortMap.clear();
	m_OutputPortMap.clear();
	
	recording_ = std::make_shared<RecordingRings>();
	audio_mix_ring = recording_->mix.get();
	first = recording_->input.get();
	m_SampleRate = jack_get_sample_rate(m_Client);
	m_BufferSize = jack_get_buffer_size(m_Client);
	m_Attached = true;
	if (!audio_mix_ring || !first || jack_activate(m_Client)) {
	  error("cannot activate JACK client");
	  detach_locked();
	  return false;
	}
	++session_;
	return m_Attached.load();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void JackClient::Detach()
{
  dispatch_shutdown();
	std::lock_guard<std::mutex> inputs(input_mutex_);
	std::lock_guard<std::mutex> lock(output_mutex_);
	detach_locked();
}

void JackClient::detach_locked()
{
	if (m_Client)
	{
	  act("Detaching from JACK");
		server_request([] { return jack_client_close(m_Client); });
		m_Client=NULL;
		m_Attached=false;
	}
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
	// JACK close has stopped callback execution before retiring the published ports.
	output_port_count_.store(0, std::memory_order_relaxed);
	
	// tells ssm to go back to non callback mode
	//if (RunCallback) RunCallback(RunContext, false);
}

/////////////////////////////////////////////////////////////////////////////////////////////

extern "C" {
#include <jack/jack.h>
// m_inbuf, out, channels, j, nframes
void deinterleave(void * _in, jack_default_audio_sample_t *out
		, int num_channels, int channel, int num_samples)
{
  int j;
  float * in;
  in = ((float*)_in) + channel;
  for(j = 0; j < num_samples; j++)
  {
    out[j] = (*in);
    in += num_channels;
  }
}

} // end extern "C"

/*
int JackClient::Process (jack_nframes_t nframes, void *arg)
{
	jack_port_t* input_port = m_InputPortMap[0]->Port;
	jack_port_t* output_port = m_OutputPortMap[0]->Port;
      jack_default_audio_sample_t *out = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port, nframes);
      jack_default_audio_sample_t *in = (jack_default_audio_sample_t *) jack_port_get_buffer (input_port, nframes);

      memcpy (out, in, sizeof (jack_default_audio_sample_t) * nframes);
      
      return 0;      
}
*/


int JackClient::Process(jack_nframes_t nframes, void *self)
{	
  auto &client = *static_cast<JackClient *>(self);
  // The single JACK reader marks the exact interval in which input handles
  // may be dereferenced. Sequential consistency orders entry against a
  // control-side list swap and its subsequent observation of this epoch.
  client.input_epoch_.fetch_add(1);
  const auto &inputs = client.input_lists_[client.active_inputs_.load()];
  const auto rate = m_SampleRate.load(std::memory_order_relaxed);
  const unsigned recording_rate = client.recording_->rate.load(std::memory_order_acquire);
  const bool recording = recording_rate ? recording_rate == rate : client.m_Encoded.load(std::memory_order_relaxed);
  const auto frame = jack_last_frame_time(m_Client);
  bool first_connected = true;
  for (size_t i = 0; i < inputs.count; ++i) {
    auto &port = *inputs.ports[i];
    const float *in = jack_port_connected(port.Port)
        ? static_cast<const float *>(jack_port_get_buffer(port.Port, nframes)) : nullptr;
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
	  if (frame_bytes && client.m_ringbuffer && period_bytes <= client.output_buffer_.size()) {
	    auto *samples = client.output_buffer_.data();
	    const auto bytes = std::min(period_bytes, ringbuffer_read_space(client.m_ringbuffer));
	    const auto received = ringbuffer_read(client.m_ringbuffer, samples, bytes - bytes % frame_bytes);
	    if (received) {
	      // The encoder receives complete source samples, including the tail
	      // of a clip. Playback alone gets zero filling to the JACK period.
	      if (recording && (!recording_rate || client.recording_->channels.load() == static_cast<unsigned>(channels))
	          && ringbuffer_write_space(client.audio_mix_ring) >= received)
	        ringbuffer_write(client.audio_mix_ring, samples, received);
	      memset(samples + received, 0, period_bytes - received);
	      output_available = true;
	    }
	  }

	  // Port slots are append-only while JACK can call Process. An acquired
	  // count makes their handles visible even when the route lock is held.
	  const int ports = client.output_port_count_.load(std::memory_order_acquire);
	  for (int channel = 0; channel < ports; ++channel) {
	    auto *output = static_cast<sample_t *>(jack_port_get_buffer(client.output_ports_[channel], nframes));
	    if (!output)
	      continue;
	    if (output_available && channel < channels)
	      deinterleave(client.output_buffer_.data(), output, channels, channel, nframes);
	    else
	      memset(output, 0, nframes * sizeof(*output));
	  }
	}
	m_BufferSize.store(nframes, std::memory_order_relaxed);
	return 0;
}

int JackClient::SetRingbufferPtr(ringbuffer_t *rb, int, int channels) {
	if (!rb || channels <= 0 || channels > MAX_OUTPUTPORTS)
	  return EINVAL;
	std::lock_guard<std::mutex> lock(output_mutex_);
	if (!m_Attached)
	  return EINVAL;
	while (output_port_count_.load(std::memory_order_relaxed) < channels) {
	  if (add_output_port_locked() < 0)
	    return ENODEV;
	}
	const std::size_t period = std::max(4096u, jack_get_buffer_size(m_Client));
	output_buffer_.resize(period * channels * sizeof(float));
	ringbuffer_reset(rb);
	m_ringbufferchannels = channels;
	m_ringbuffer = rb;
	return 0;
}

void JackClient::ResetRingbuffer(ringbuffer_t *rb, bool disconnect) {
	if (auto *client = m_Singleton) {
	  std::lock_guard<std::mutex> lock(client->output_mutex_);
	  if (client->m_ringbuffer == rb) {
	    ringbuffer_reset(rb);
	    if (disconnect) {
	      client->m_ringbuffer = nullptr;
	      // Preserve the recording format until a different supplier binds.
	    }
	  }
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////

int JackClient::AddInputPort(std::shared_ptr<AudioCapture> capture)
{
  std::lock_guard<std::mutex> lock(input_mutex_);
  if (!m_Attached || m_InputPortMap.size() == MAX_INPUTPORTS) return -1;
  std::unique_ptr<JackPort> port(new JackPort);
  const int id = m_NextInputID++;
  port->Name = "In" + std::to_string(id);
  port->Port = jack_port_register(m_Client, port->Name.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
  if (!port->Port) return -1;
  port->capture = std::move(capture);
  m_InputPortMap[id] = port.release();
  publish_inputs_locked();
  return id;
}

void JackClient::publish_inputs_locked()
{
  const unsigned next = 1 - active_inputs_.load();
  auto &list = input_lists_[next];
  list.count = 0;
  for (const auto &entry : m_InputPortMap) list.ports[list.count++] = entry.second;
  active_inputs_.store(next);

  // Readers starting after the swap cannot acquire the old list. Only
  // wait for the reader already in flight, never for the next JACK period.
  const auto epoch = input_epoch_.load();
  if (epoch & 1u) {
    while (input_epoch_.load() == epoch) std::this_thread::yield();
  }
}

void JackClient::RemoveInputPort(int id)
{
  std::lock_guard<std::mutex> lock(input_mutex_);
  auto it = m_InputPortMap.find(id);
  if (it == m_InputPortMap.end()) return;
  std::unique_ptr<JackPort> port(it->second);
  m_InputPortMap.erase(it);
  publish_inputs_locked();
  if (m_Attached && server_request([&] { return jack_port_unregister(m_Client, port->Port); }))
    error("Could not unregister JACK input %s", port->Name.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////

int JackClient::AddOutputPort()
{
	std::lock_guard<std::mutex> lock(output_mutex_);
	return add_output_port_locked();
}

int JackClient::add_output_port_locked()
{
	static_assert(ATOMIC_INT_LOCK_FREE == 2, "JACK output publication must be lock-free");
	const int channel = output_port_count_.load(std::memory_order_relaxed);
	if (!m_Attached || channel == MAX_OUTPUTPORTS)
	  return -1;

	char name[32];
	snprintf(name, sizeof(name), "Out%d", channel);
	auto *port = new JackPort;
	port->Name = name;
	port->Port = jack_port_register(m_Client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
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

int JackClient::OnSRateChange(jack_nframes_t n, void *)
{
	m_SampleRate=n;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void JackClient::OnJackShutdown(void *o)
{
  // JACK's shutdown thread only publishes state. Deliver an optional user
  // notification from the next control-thread status, close or reopen call.
  auto &client = *static_cast<JackClient *>(o);
  m_Attached.store(false);
  client.shutdown_pending_.store(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void JackClient::GetPortNames(std::vector<std::string> &InputNames, std::vector<std::string> &OutputNames)
{
  std::lock_guard<std::mutex> lock(output_mutex_);
  InputNames.clear();
  OutputNames.clear();
  if (!m_Attached) return;
  for (const auto flag : {JackPortIsInput, JackPortIsOutput}) {
    const char **names = jack_get_ports(m_Client, nullptr, nullptr, flag);
    if (!names) continue;
    auto &result = flag == JackPortIsInput ? InputNames : OutputNames;
    for (size_t n = 0; names[n]; ++n) result.emplace_back(names[n]);
    jack_free(names);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Input means input of SSM, so this connects jack sources to the plugin outputs
void JackClient::ConnectInput(int n, const std::string &JackPort)
{
  std::lock_guard<std::mutex> lock(input_mutex_);
  if (!m_Attached || !m_InputPortMap.count(n)) return;
  
  //cerr<<"JackClient::ConnectInput: connecting source ["<<JackPort<<"] to dest ["<<m_InputPortMap[n]->Name<<"]"<<endl;
  
  if (m_InputPortMap[n]->ConnectedTo!="")
    {
      if (jack_disconnect (m_Client, m_InputPortMap[n]->ConnectedTo.c_str(), jack_port_name(m_InputPortMap[n]->Port)))
	error("Audio Jack ConnectInput: cannot disconnect input port [%s] from [%s]",
	      m_InputPortMap[n]->ConnectedTo.c_str(), m_InputPortMap[n]->Name.c_str());
    }
  
  m_InputPortMap[n]->ConnectedTo = JackPort;
  
  if (jack_connect (m_Client, JackPort.c_str(),
		    jack_port_name(m_InputPortMap[n]->Port)))

    error("JackClient::ConnectInput: cannot connect input port [%s] to [%s]",
	  JackPort.c_str(), m_InputPortMap[n]->Name.c_str());
  
	m_InputPortMap[n]->Connected=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Output means output of SSM, so this connects plugin inputs to a jack destination
void JackClient::ConnectOutput(int n, const std::string &JackPort)
{
  std::lock_guard<std::mutex> lock(output_mutex_);
  if (!m_Attached || !m_OutputPortMap.count(n)) return;
  std::cerr<<"JackClient::ConnectOutput: connecting source ["<<m_OutputPortMap[n]->Name<<"] \
  	to dest ["<<JackPort<<"]"<<std::endl;
  
  if (m_OutputPortMap[n]->ConnectedTo!="")
    {
      if (jack_disconnect (m_Client, jack_port_name(m_OutputPortMap[n]->Port), m_OutputPortMap[n]->ConnectedTo.c_str()))
	error("JackClient::ConnectOutput: cannot disconnect output port [%s] to [%s]",
	      m_OutputPortMap[n]->ConnectedTo.c_str(),
	      m_OutputPortMap[n]->Name.c_str());
    }
  
  m_OutputPortMap[n]->ConnectedTo = JackPort;
  if (jack_connect (m_Client, jack_port_name(m_OutputPortMap[n]->Port), JackPort.c_str()))
    error("JackClient::ConnectOutput: cannot connect output port [%s] to [%s]",
	  m_OutputPortMap[n]->Name.c_str(), JackPort.c_str());
  m_OutputPortMap[n]->Connected=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Input means input of SSM, so this connects jack sources to the plugin outputs
void JackClient::DisconnectInput(int n)
{
  std::lock_guard<std::mutex> lock(input_mutex_);
  if (!m_Attached || !m_InputPortMap.count(n)) return;
	//cerr<<"JackClient::DisconnectInput: Disconnecting input "<<n<<endl;

  if (m_InputPortMap[n]->ConnectedTo!="")
    {
      if (jack_disconnect (m_Client, m_InputPortMap[n]->ConnectedTo.c_str(), jack_port_name(m_InputPortMap[n]->Port)))
	error("JackClient::ConnectInput: cannot disconnect input port [%s] from [%s]",
	      m_InputPortMap[n]->ConnectedTo.c_str(),
	      m_InputPortMap[n]->Name.c_str());
    }
  
  m_InputPortMap[n]->Connected=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Output means output of SSM, so this connects plugin inputs to a jack destination
void JackClient::DisconnectOutput(int n)
{
  std::lock_guard<std::mutex> lock(output_mutex_);
  if (!m_Attached || !m_OutputPortMap.count(n)) return;
	//cerr<<"JackClient::DisconnectInput: Disconnecting input "<<n<<endl;

	if (m_OutputPortMap[n]->ConnectedTo!="")
	{
	  if (jack_disconnect (m_Client, jack_port_name(m_OutputPortMap[n]->Port), m_OutputPortMap[n]->ConnectedTo.c_str()))
	    error("JackClient::ConnectOutput: cannot disconnect output port [%s] from [%s]",
		  m_OutputPortMap[n]->ConnectedTo.c_str(),
		  m_OutputPortMap[n]->Name.c_str());
	}

	m_OutputPortMap[n]->Connected=false;
}
/////////////////////////////////////////////////////////////////////////////////////////////

void JackClient::SetInputBuf(int ID, float* s)
{
  std::lock_guard<std::mutex> lock(input_mutex_);
	if(m_InputPortMap.find(ID)!=m_InputPortMap.end()) m_InputPortMap[ID]->Buf=s;
	else std::cerr<<"Could not find port ID "<<ID<<std::endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////
	
void JackClient::SetOutputBuf(int ID, float* s)
{
  std::lock_guard<std::mutex> lock(output_mutex_);
	if(m_OutputPortMap.find(ID)!=m_OutputPortMap.end()) m_OutputPortMap[ID]->Buf=s;
	else error("Could not find port ID %u", ID);
}


#endif
