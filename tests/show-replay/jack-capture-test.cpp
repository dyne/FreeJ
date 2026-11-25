#include <audio_collector.h>
#include <audio_jack.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#ifdef __linux__
#include <dirent.h>
#endif
#include <cstring>
#include <thread>

namespace {
struct Port { std::array<float, 256> pcm{}; bool connected = true, registered = false; };
std::array<Port, 4> ports;
unsigned count, frame = UINT32_MAX - 15, rate = 48000;
int server;
JackProcessCallback process;
JackSampleRateCallback change_rate;
JackShutdownCallback shutdown_event;
void *engine, *rate_context, *shutdown_context;
int checks;
std::atomic<bool> hold_input{false}, inside_input{false}, hold_unregister{false}, inside_unregister{false};
bool wait_for(const std::atomic<bool> &flag) {
  auto until = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (!flag.load() && std::chrono::steady_clock::now() < until) std::this_thread::yield();
  return flag.load();
}
void check(bool ok, const char *name) {
  ++checks;
  if (!ok) {
    std::fprintf(stderr, "FAIL %s at %d\n", name, checks);
    std::exit(1);
  }
}
void period(unsigned n, float value) {
  ports[0].pcm.fill(value);
  check(process(n, engine) == 0, "device callback");
  frame += n;
}
bool wait_window(AudioCollector &audio, uint32_t first, AudioCapture::Position &p) {
  auto limit = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  do {
    audio.GetFFT();
    if (audio.CapturePosition(p) && p.first == first) return true;
    std::this_thread::yield();
  } while (std::chrono::steady_clock::now() < limit);
  return false;
}
#ifdef __linux__
int descriptor_count() {
  DIR *dir = opendir("/proc/self/fd");
  if (!dir) return -1;
  int entries = 0;
  while (readdir(dir)) ++entries;
  closedir(dir);
  return entries;
}
#endif

}

void *(*jmemcpy)(void *, const void *, size_t) = std::memcpy;
void act(const char *, ...) {}
void error(const char *, ...) {}
void func(const char *, ...) {}
extern "C" {
jack_client_t *jack_client_open(const char *, jack_options_t, jack_status_t *, ...) {
  return reinterpret_cast<jack_client_t *>(&server);
}
int jack_set_process_callback(jack_client_t *, JackProcessCallback fn, void *p) {
  process = fn; engine = p; return 0;
}
int jack_set_sample_rate_callback(jack_client_t *, JackSampleRateCallback fn, void *p) {
  change_rate = fn; rate_context = p; return 0;
}
void jack_on_shutdown(jack_client_t *, JackShutdownCallback fn, void *p) {
  shutdown_event = fn; shutdown_context = p;
}
int jack_activate(jack_client_t *) { return 0; }
int jack_client_close(jack_client_t *) {
  count = 0;
  for (auto &port : ports) port.registered = false;
  return 0;
}
jack_port_t *jack_port_register(jack_client_t *, const char *, const char *, unsigned long, unsigned long) {
  for (auto &port : ports) {
    if (port.registered) continue;
    port = {};
    port.registered = true;
    ++count;
    return reinterpret_cast<jack_port_t *>(&port);
  }
  return nullptr;
}
int jack_port_unregister(jack_client_t *, jack_port_t *handle) {
  auto &port = *reinterpret_cast<Port *>(handle);
  if (!port.registered) return -1;
  if (hold_unregister.load()) {
    inside_unregister = true;
    while (hold_unregister.load()) std::this_thread::yield();
  }
  port.registered = false;
  --count;
  return 0;
}
void *jack_port_get_buffer(jack_port_t *p, jack_nframes_t) {
  auto *port = reinterpret_cast<Port *>(p);
  if (port == &ports[1] && hold_input.load()) {
    inside_input = true;
    while (hold_input.load()) std::this_thread::yield();
  }
  return port->pcm.data();
}
jack_nframes_t jack_get_buffer_size(jack_client_t *) { return 4; }
jack_nframes_t jack_get_sample_rate(jack_client_t *) { return rate; }
jack_nframes_t jack_last_frame_time(const jack_client_t *) { return frame; }
const char **jack_get_ports(jack_client_t *, const char *, const char *, unsigned long) { return nullptr; }
void jack_free(void *p) { std::free(p); }
int jack_port_connected(const jack_port_t *p) { return reinterpret_cast<const Port *>(p)->connected; }
const char *jack_port_name(const jack_port_t *) { return "fixture"; }
int jack_connect(jack_client_t *, const char *, const char *) { return 0; }
int jack_disconnect(jack_client_t *, const char *, const char *) { return 0; }
}

int main() {
  char source[] = "stage:input";
  auto *jack = JackClient::Get();
  auto original = std::unique_ptr<AudioCollector>(new AudioCollector(source, 16, 44100));
  AudioCapture::Position p;
  const auto first = frame;
  for (int i = 0; i < 3; ++i) {
    period(4, 1);
    check(original->GetFFT()[0] == 0 && !original->CapturePosition(p), "incomplete window");
  }
  period(4, 1);
  check(wait_window(*original, first, p) && original->GetHarmonic(0) > 0 &&
        p.rate == 48000 && p.period == 4 && frame == 0, "published at frame counter rollover");
  check(ringbuffer_read_space(jack->first) == 0, "recording off, analysis on");
  auto recording = jack->BeginRecording();
  check(recording && !jack->BeginRecording(), "one recording consumer");
  for (int i = 0; i < 500; ++i) period(16, float(i));
  check(wait_window(*original, frame - 16, p) &&
        original->GetAudioBuffer()[0] == 499, "slow analysis reads latest complete window");
  check(ringbuffer_read_space(recording->input.get()) == 500 * 16 * sizeof(float),
        "analysis and recording have independent consumption positions");
  std::vector<float> saved(500 * 16);
  ringbuffer_read(recording->input.get(), reinterpret_cast<char *>(saved.data()), saved.size() * sizeof(float));
  for (size_t i = 0; i < saved.size(); ++i) check(saved[i] == i / 16, "recorded samples preserved");
  period(4, 1);
  check(original->GetFFT()[0] == 0 && !original->CapturePosition(p), "period change clears stale output");
  rate = 44100; change_rate(rate, rate_context);
  for (int i = 0; i < 4; ++i) period(4, 2);
  check(wait_window(*original, frame - 16, p) && p.rate == rate, "new-rate complete window");
  frame += 4;
  period(4, 1);
  check(original->GetFFT()[0] == 0 && !original->CapturePosition(p), "missing samples clear partial history");
  ports[0].connected = false;
  period(4, 1);
  ports[0].connected = true;
  for (int i = 0; i < 3; ++i) period(4, 3);
  check(original->GetFFT()[0] == 0, "disconnected samples never join a new window");
  period(4, 3);
  check(wait_window(*original, frame - 16, p) && original->GetAudioBuffer()[0] == 3, "reconnected source");
  shutdown_event(shutdown_context);
  check(!original->IsConnected() && original->GetFFT()[0] == 0, "shutdown without user callback");
  {
    AudioCollector reopened(source, 16, 44100);
    original.reset();
    check(reopened.IsConnected(), "retired collector cannot detach a later session");
    period(16, 1);
    check(wait_window(reopened, frame-16, p) && reopened.GetHarmonic(0) > 0, "new session publishes independently");
    bool notified = false;
    jack->SetCallback([](void *p, unsigned n) { *static_cast<bool *>(p) = n == 0; }, &notified);
    shutdown_event(shutdown_context);
    check(!notified && !reopened.IsConnected() && notified, "shutdown notification on control thread");
    jack->ClearCallback(&notified);
  }
  check(jack->Attach("external"), "external owner available across repeated borrowed opens");
  const int permanent = jack->AddInputPort();
  auto continuous = jack->BeginRecording();
  check(permanent >= 0 && continuous && count == 1, "unrelated input and recording active");
#ifdef __linux__
  const int descriptors = descriptor_count();
  check(descriptors >= 0, "open descriptor baseline");
#endif
  for (int i = 0; i < MAX_INPUTPORTS + 2; ++i) {
    int transient;
    {
      AudioCollector borrowed(16, 44100, jack);
      transient = borrowed.AddAnalysisInput();
      check(transient >= 0 && count == 2, "borrowed input admitted");
      jack->ConnectInput(transient, source);
      period(16, float(i));
      check(wait_window(borrowed, frame-16, p), "reopened capture publishes a complete window");
      jack->DisconnectInput(transient);
      check(!jack->GetInputName(transient).empty(), "ordinary disconnect retains the registered port");
    }
    check(count == 1 && jack->GetInputName(transient).empty(), "borrowed collector unregisters only its port");
    check(jack->IsAttached() && !jack->GetInputName(permanent).empty(), "external connection and input retained");
  }
#ifdef __linux__
  check(descriptor_count() == descriptors, "all capture worker descriptors released");
#endif
  check(ringbuffer_read_space(continuous->input.get()) == (MAX_INPUTPORTS + 2)*16*sizeof(float),
        "all unrelated recording periods retained while borrowed ports reopen");
  auto borrowed = std::unique_ptr<AudioCollector>(new AudioCollector(source, 16, 44100));
  check(count == 2, "borrowed script constructor registered the second port");
  hold_input = hold_unregister = true;
  std::thread first_period([&] { process(16, engine); });
  const bool entered = wait_for(inside_input);
  std::atomic<bool> retiring{false};
  std::thread retire([&] { retiring = true; borrowed.reset(); });
  const bool started = wait_for(retiring);
  const auto grace = std::chrono::steady_clock::now() + std::chrono::milliseconds(20);
  while (!inside_unregister.load() && std::chrono::steady_clock::now() < grace) std::this_thread::yield();
  const bool premature = inside_unregister.load();
  hold_input = false;
  first_period.join();
  const bool unregistering = wait_for(inside_unregister);
  // Unregistration can block on the JACK server. The unrelated recording
  // input must keep running even while that control operation is held.
  std::atomic<bool> finished{false};
  std::thread next_period([&] { process(16, engine); finished = true; });
  const bool continued = wait_for(finished);
  hold_unregister = false;
  next_period.join();
  retire.join();
  check(entered && started && !premature && unregistering && continued && count == 1,
        "retirement waits for a reader using the old port; further callbacks run during unregister");
  check(ringbuffer_read_space(continuous->input.get()) == (MAX_INPUTPORTS + 4)*16*sizeof(float),
        "all unrelated recording samples retained through concurrent retirement");
  JackClient::PackUpAndGoHome();
  check(recording->rate.load() == 48000, "recording storage survives device retirement");

  AudioCapture handoff(128);
  std::atomic<bool> start{false}, done{false};
  std::thread producer([&] {
    while (!start.load()) std::this_thread::yield();
    std::array<float, 128> samples;
    for (unsigned i = 0; i < 60000; ++i) {
      samples.fill(float(i));
      handoff.write(samples.data(), samples.size(), i * samples.size(), 48000);
    }
    done = true;
  });
  start = true;
  unsigned snapshots = 0;
  bool coherent = true;
  std::array<float, 128> samples;
  do {
    if (handoff.read(samples.data(), p)) {
      ++snapshots;
      coherent &= std::all_of(samples.begin(), samples.end(), [&](float v) { return v == p.first / 128; });
    }
  } while (!done.load());
  producer.join();
  if (handoff.read(samples.data(), p)) ++snapshots;
  check(coherent && snapshots && p.first == 59999 * 128, "concurrent reader never sees a torn window");
  RecordingRings::Ring ring(ringbuffer_create(64), ringbuffer_free);
  uint32_t sequence = 0;
  check(ring && !ringbuffer_read(ring.get(), reinterpret_cast<char *>(&sequence), sizeof(sequence)),
        "empty recording ring underrun returns zero");
  std::atomic<bool> abandon{false};
  std::thread writer([&] {
    for (uint32_t i = 0; i < 20000 && !abandon.load();) {
      if (ringbuffer_write_space(ring.get()) >= sizeof(i)) {
        ringbuffer_write(ring.get(), reinterpret_cast<const char *>(&i), sizeof(i));
        ++i;
      } else std::this_thread::yield();
    }
  });
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  uint32_t expected = 0;
  while (expected < 20000 && std::chrono::steady_clock::now() < deadline) {
    if (ringbuffer_read_space(ring.get()) < sizeof(sequence)) { std::this_thread::yield(); continue; }
    ringbuffer_read(ring.get(), reinterpret_cast<char *>(&sequence), sizeof(sequence));
    if (sequence != expected) break;
    ++expected;
  }
  abandon = true;
  writer.join();
  check(expected == 20000, "20000 SPSC recording values in order across wrap and full ring");
  std::printf("PASS %d checks; %u coherent snapshots from 60000 windows\n", checks, snapshots);
}
