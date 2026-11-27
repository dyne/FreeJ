#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdarg>
#include <cstring>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>

#include <audio_pipewire.h>
#include "audio-graph-fixture.h"

namespace {

constexpr unsigned period = 4;
using Fixture = AudioGraphFixture;

bool matches(unsigned port, std::array<float, period> samples) {
  return std::equal(samples.begin(), samples.end(), Fixture::ports[port].pcm.begin());
}
std::mutex gate_mutex;
std::condition_variable gate;
bool hold_route = false;
bool route_entered = false;
unsigned underrun_reports = 0, underrun_summary = 0;

bool play(ringbuffer_t *source) {
  const float stereo[] = {1, -1, 2, -2, 3, -3, 4, -4};
  return ringbuffer_write(source, reinterpret_cast<const char *>(stereo), sizeof(stereo)) ==
             sizeof(stereo) && Fixture::cycle(period) == 0 &&
         matches(0, {1, 2, 3, 4}) &&
         matches(1, {-1, -2, -3, -4});
}

}

void *(*jmemcpy)(void *, const void *, std::size_t) = std::memcpy;
void act(const char *, ...) {}
void error(const char *, ...) {}
void func(const char *format, ...) {
  if (std::strstr(format, "playback queue underrun at"))
    ++underrun_reports;
  if (std::strstr(format, "playback queue underrun summary")) {
    va_list args;
    va_start(args, format);
    underrun_summary += va_arg(args, unsigned);
    va_end(args);
  }
}

int main() {
  using namespace std::chrono;
  Fixture::connecting = [] {
    std::unique_lock<std::mutex> lock(gate_mutex);
    if (!hold_route) return;
    route_entered = true;
    gate.notify_all();
    gate.wait(lock, [] { return !hold_route; });
  };
  using Ring = std::unique_ptr<ringbuffer_t, decltype(&ringbuffer_free)>;
  Ring first(ringbuffer_create(256), ringbuffer_free);
  Ring second(ringbuffer_create(256), ringbuffer_free);
  auto *jack = PipeWireClient::Get();
  if (!first || !second || !jack->Attach("test") ||
      jack->SetRingbufferPtr(first.get(), 48000, 2) != 0 || !play(first.get())) {
    PipeWireClient::PackUpAndGoHome();
    std::cerr << "FAIL setup or preceding nonzero stereo period\n";
    return 1;
  }

  {
    std::lock_guard<std::mutex> lock(gate_mutex);
    hold_route = true;
  }
  auto control = std::async(std::launch::async, [&] { jack->ConnectOutput(0, "test:sink"); return jack->SetRingbufferPtr(second.get(), 48000, 2); });
  bool entered = false;
  {
    std::unique_lock<std::mutex> lock(gate_mutex);
    entered = gate.wait_for(lock, seconds(2), [] { return route_entered; });
  }
  bool settled = false;
  bool silent = false;
  std::future<int> callback;
  if (entered) {
    callback = std::async(std::launch::async, [] { return Fixture::cycle(period); });
    settled = callback.wait_for(seconds(2)) == std::future_status::ready;
    if (settled) {
      const std::array<sample_t, period> silence{};
      silent = matches(0, silence) && matches(1, silence);
    }
  }
  // Always release the route writer before joining, even on a failing assertion.
  {
    std::lock_guard<std::mutex> lock(gate_mutex);
    hold_route = false;
  }
  gate.notify_all();
  const bool callback_ok = callback.valid() && callback.get() == 0;
  const bool resumed = control.get() == 0 && play(second.get());

  jack->SetCallback([](void *, unsigned) {}, nullptr);
  Fixture::shutdown(Fixture::graph);
  const bool reattached = jack->Attach("reattached") && Fixture::count == 0 &&
      jack->SetRingbufferPtr(first.get(), 48000, 2) == 0 &&
      Fixture::count == 2 && play(first.get());
  for (int i = 0; i < 3; ++i) {
    Fixture::cycle(period);
    PipeWireClient::ReportPlaybackUnderruns();
  }
  const bool reported_once = underrun_reports == 1 && underrun_summary == 0;
  PipeWireClient::PackUpAndGoHome();
  if (!reported_once || underrun_summary != 3) {
    std::cerr << "FAIL playback underrun diagnostics: reports=" << underrun_reports
              << " total=" << underrun_summary << '\n';
    return 1;
  }
  std::cout << "PASS three PCM underruns reported in one episode with a final count\n";

  if (!entered || !settled || !silent || !callback_ok || !resumed || !reattached) {
    std::cerr << "FAIL PipeWire contention: entered=" << entered << " settled_while_locked=" << settled
              << " both_outputs_silent=" << silent << " callback_ok=" << callback_ok
              << " resumed=" << resumed << " reattached=" << reattached << '\n';
    return 1;
  }
  std::cout << "PASS nonblocking PipeWire contention clears both prior output buffers; playback resumes and ports republish after PipeWire reattach\n";
}
