#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>

#include <audio_jack.h>

namespace {

constexpr jack_nframes_t period = 4;

struct TestOutput {
  std::array<sample_t, period> samples{};
};

std::array<TestOutput, 2> outputs;
std::size_t registered = 0;
JackProcessCallback process = nullptr;
void *process_data = nullptr;
JackShutdownCallback shutdown_client = nullptr;
int client_handle;
std::mutex gate_mutex;
std::condition_variable gate;
bool hold_route = false;
bool route_entered = false;

bool play(ringbuffer_t *source) {
  const float stereo[] = {1, -1, 2, -2, 3, -3, 4, -4};
  return ringbuffer_write(source, reinterpret_cast<const char *>(stereo), sizeof(stereo)) ==
             sizeof(stereo) && process(period, process_data) == 0 &&
         outputs[0].samples == std::array<sample_t, period>{1, 2, 3, 4} &&
         outputs[1].samples == std::array<sample_t, period>{-1, -2, -3, -4};
}

}

void *(*jmemcpy)(void *, const void *, std::size_t) = std::memcpy;
void act(const char *, ...) {}
void error(const char *, ...) {}
void func(const char *, ...) {}

extern "C" {

jack_client_t *jack_client_open(const char *, jack_options_t, jack_status_t *, ...) {
  return reinterpret_cast<jack_client_t *>(&client_handle);
}

int jack_set_process_callback(jack_client_t *, JackProcessCallback callback, void *data) {
  process = callback;
  process_data = data;
  return 0;
}

int jack_set_sample_rate_callback(jack_client_t *, JackSampleRateCallback, void *) { return 0; }
void jack_on_shutdown(jack_client_t *, JackShutdownCallback callback, void *) { shutdown_client = callback; }
int jack_activate(jack_client_t *) { return 0; }
int jack_client_close(jack_client_t *) { registered = 0; return 0; }

jack_port_t *jack_port_register(jack_client_t *, const char *, const char *, unsigned long,
                                unsigned long) {
  return registered < outputs.size() ? reinterpret_cast<jack_port_t *>(&outputs[registered++])
                                     : nullptr;
}

void *jack_port_get_buffer(jack_port_t *port, jack_nframes_t) {
  // Like the real JACK output API, return period storage with its prior contents.
  return reinterpret_cast<TestOutput *>(port)->samples.data();
}

jack_nframes_t jack_get_buffer_size(jack_client_t *) {
  // SetRingbufferPtr calls this while holding the route lock.
  std::unique_lock<std::mutex> lock(gate_mutex);
  if (hold_route) {
    route_entered = true;
    gate.notify_all();
    gate.wait(lock, [] { return !hold_route; });
  }
  return period;
}

const char **jack_get_ports(jack_client_t *, const char *, const char *, unsigned long) {
  return nullptr;
}
int jack_port_connected(const jack_port_t *) { return 0; }
const char *jack_port_name(const jack_port_t *) { return "test"; }
int jack_connect(jack_client_t *, const char *, const char *) { return 0; }
int jack_disconnect(jack_client_t *, const char *, const char *) { return 0; }

}

int main() {
  using namespace std::chrono;
  using Ring = std::unique_ptr<ringbuffer_t, decltype(&ringbuffer_free)>;
  Ring first(ringbuffer_create(256), ringbuffer_free);
  Ring second(ringbuffer_create(256), ringbuffer_free);
  auto *jack = JackClient::Get();
  if (!first || !second || !jack->Attach("test") || !process || !shutdown_client ||
      jack->SetRingbufferPtr(first.get(), 48000, 2) != 0 || !play(first.get())) {
    JackClient::PackUpAndGoHome();
    std::cerr << "FAIL setup or preceding nonzero stereo period\n";
    return 1;
  }

  {
    std::lock_guard<std::mutex> lock(gate_mutex);
    hold_route = true;
  }
  auto control = std::async(std::launch::async, [&] { return jack->SetRingbufferPtr(second.get(), 48000, 2); });
  bool entered = false;
  {
    std::unique_lock<std::mutex> lock(gate_mutex);
    entered = gate.wait_for(lock, seconds(2), [] { return route_entered; });
  }
  bool settled = false;
  bool silent = false;
  std::future<int> callback;
  if (entered) {
    callback = std::async(std::launch::async, [] { return process(period, process_data); });
    settled = callback.wait_for(seconds(2)) == std::future_status::ready;
    if (settled) {
      const std::array<sample_t, period> silence{};
      silent = outputs[0].samples == silence && outputs[1].samples == silence;
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
  shutdown_client(nullptr);
  const bool reattached = jack->Attach("reattached") && registered == 0 &&
      jack->SetRingbufferPtr(first.get(), 48000, 2) == 0 &&
      registered == 2 && play(first.get());
  JackClient::PackUpAndGoHome();

  if (!entered || !settled || !silent || !callback_ok || !resumed || !reattached) {
    std::cerr << "FAIL JACK contention: entered=" << entered << " settled_while_locked=" << settled
              << " both_outputs_silent=" << silent << " callback_ok=" << callback_ok
              << " resumed=" << resumed << " reattached=" << reattached << '\n';
    return 1;
  }
  std::cout << "PASS nonblocking JACK contention clears both prior output buffers; playback resumes and ports republish after JACK reattach\n";
}
