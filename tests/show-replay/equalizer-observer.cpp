// Diagnostic preload only. Copy one actual PipeWire input period per script FFT
// call. Its synchronization and copying are deliberately outside timing runs.
#include <audio_collector.h>
#include "pipewire-observer.h"
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
pw_filter_events events;
void (*process)(void *, spa_io_position *);
void *process_context;
bool copied = false;
bool injected() {
  const char *route = std::getenv("FREEJ_EQUALIZER_ROUTE");
  return route && std::strcmp(route, "injected") == 0;
}
void observe_process(void *, spa_io_position *position) {
  copied = false;
  process(process_context, position);
  if (copied) {
    std::lock_guard<std::mutex> lock(gate);
    requested = false;
    received.notify_one();
  }
}
}

extern "C" void pw_filter_add_listener(pw_filter *filter, spa_hook *hook,
                                      const pw_filter_events *original, void *data) {
  static auto add = reinterpret_cast<decltype(&pw_filter_add_listener)>(
      dlsym(RTLD_NEXT, "pw_filter_add_listener"));
  if (original && original->process) {
    process = original->process;
    process_context = data;
    events = *original;
    events.process = observe_process;
    original = &events;
  }
  add(filter, hook, original, data);
}

extern "C" void *pw_filter_get_dsp_buffer(void *port, uint32_t frames) {
  static auto get = reinterpret_cast<decltype(&pw_filter_get_dsp_buffer)>(
      dlsym(RTLD_NEXT, "pw_filter_get_dsp_buffer"));
  auto *samples = static_cast<float *>(get(port, frames));
  if (samples && port == observed_input.load(std::memory_order_acquire)) {
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
    std::fputs("no known PipeWire input period\n", stderr);
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
