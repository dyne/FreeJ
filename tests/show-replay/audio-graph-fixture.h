// Deterministic graph controls for the production audio callback tests.
#ifndef FREEJ_AUDIO_GRAPH_FIXTURE_H
#define FREEJ_AUDIO_GRAPH_FIXTURE_H

#include <spa/node/io.h>
#include <pipewire/pipewire.h>

struct AudioGraphFixture {
  struct Port {
    std::array<float, 256> pcm{};
    std::atomic<bool> *linked = nullptr;
  };
  static std::array<Port, 4> ports;
  static unsigned count;
  static void (*process)(void *, spa_io_position *);
  static void (*shutdown)(void *);
  static void *engine, *graph;
  static void (*reading)(Port *);
  static void (*removing)();
  static void (*connecting)();

  static int cycle(unsigned frames, uint32_t frame = 0, unsigned rate = 48000)
  {
    spa_io_position position{};
    position.clock.duration = frames;
    position.clock.position = frame;
    position.clock.rate = SPA_FRACTION(1, rate);
    process(engine, &position);
    return 0;
  }
};

#ifdef FREEJ_TEST_AUDIO_GRAPH_IMPL
std::array<AudioGraphFixture::Port, 4> AudioGraphFixture::ports;
unsigned AudioGraphFixture::count;
void (*AudioGraphFixture::process)(void *, spa_io_position *);
void (*AudioGraphFixture::shutdown)(void *);
void *AudioGraphFixture::engine, *AudioGraphFixture::graph;
void (*AudioGraphFixture::reading)(Port *);
void (*AudioGraphFixture::removing)();
void (*AudioGraphFixture::connecting)();

struct PipeWireClient::Graph : AudioGraphFixture {
  explicit Graph(PipeWireClient &client) : client(client) {}
  ~Graph() {
    count = 0;
    for (auto &port : ports) port = {};
  }
  bool open(const std::string &) {
    process = &PipeWireClient::Process;
    engine = &client;
    graph = this;
    shutdown = [](void *data) {
      auto &client = static_cast<Graph *>(data)->client;
      m_Attached = false;
      client.shutdown_pending_ = true;
    };
    m_SampleRate = 48000;
    m_BufferSize = 4;
    m_Attached = true;
    return true;
  }
  void *add_port(AudioPort &port, pw_direction) {
    for (auto &slot : ports) {
      if (slot.linked) continue;
      slot = {};
      slot.linked = &port.linked;
      port.linked = true;
      ++count;
      return &slot;
    }
    return nullptr;
  }
  void remove_port(AudioPort &port) {
    if (removing) removing();
    static_cast<Port *>(port.Port)->linked = nullptr;
    --count;
  }
  static float *buffer(void *port, unsigned) {
    auto *slot = static_cast<Port *>(port);
    if (reading) reading(slot);
    return slot->pcm.data();
  }
  void connect(AudioPort &port, bool, const std::string &peer) {
    if (connecting) connecting();
    port.linked = !peer.empty();
  }
  void names(std::vector<std::string> &, std::vector<std::string> &) {}

  PipeWireClient &client;
};
#endif
#endif
