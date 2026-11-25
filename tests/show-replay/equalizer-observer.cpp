// Diagnostic preload only. Copy one actual JACK input period per script FFT
// call. Its synchronization and copying are deliberately outside timing runs.
#include <audio_collector.h>
#include <jack/jack.h>
#include <dlfcn.h>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {
std::mutex gate;
std::condition_variable received;
bool requested = false;
std::vector<float> period;
JackProcessCallback process;
void *process_context;
bool copied = false;
bool injected() {
  const char *route = std::getenv("FREEJ_EQUALIZER_ROUTE");
  return route && std::strcmp(route, "injected") == 0;
}
int observe_process(jack_nframes_t frames, void *) {
  copied = false;
  const int result = process(frames, process_context);
  if (copied) {
    std::lock_guard<std::mutex> lock(gate);
    requested = false;
    received.notify_one();
  }
  return result;
}
}

extern "C" int jack_set_process_callback(jack_client_t *client, JackProcessCallback call, void *context) {
  static auto set = reinterpret_cast<decltype(&jack_set_process_callback)>(
      dlsym(RTLD_NEXT, "jack_set_process_callback"));
  process = call;
  process_context = context;
  return set(client, observe_process, nullptr);
}

extern "C" void *jack_port_get_buffer(jack_port_t *port, jack_nframes_t frames) {
  using Get = void *(*)(jack_port_t *, jack_nframes_t);
  static auto get = reinterpret_cast<Get>(dlsym(RTLD_NEXT, "jack_port_get_buffer"));
  auto *samples = static_cast<float *>(get(port, frames));
  if (samples && (jack_port_flags(port) & JackPortIsInput)) {
    std::lock_guard<std::mutex> lock(gate);
    if (requested && samples[0] == 0.5f) {
      period.assign(samples, samples + frames);
      copied = true;
    }
    // For injection, retain the real input block above and withhold it from
    // the live port so this test never feeds it twice.
    if (injected()) return nullptr;
  }
  return samples;
}

float *AudioCollector::GetFFT() {
  using Get = float *(*)(AudioCollector *);
  static auto get = reinterpret_cast<Get>(dlsym(RTLD_NEXT, "_ZN14AudioCollector6GetFFTEv"));
  std::unique_lock<std::mutex> lock(gate);
  requested = true;
  if (!received.wait_for(lock, std::chrono::seconds(3), [] { return !requested; })) {
    std::fputs("no known JACK input period\n", stderr);
    std::_Exit(2);
  }
  if (injected())
    SubmitSamples(period.data(), period.size());
  std::fprintf(stderr, "equalizer: capture %zu %.6f %.6f\n", period.size(), period[0], period[1]);
  lock.unlock();
  auto *result = get(this);
  AudioCapture::Position position;
  const bool complete = CapturePosition(position);
  std::fprintf(stderr, "equalizer: window %d %u %u %u\n", complete, position.first,
               position.period, position.rate);
  return result;
}
