#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <unistd.h>

#include <context.h>
#include <fastmemcpy.h>
#include <oggtheora_encoder.h>
#include <soft_screen.h>

#include "frame_storage.h"

namespace freej_engine {
struct WorkerAccess {
  static int fail_start(JSyncThread &worker) {
    return worker.start_with([](pthread_t *, const pthread_attr_t *, void *(*)(void *), void *) {
      return EAGAIN;
    });
  }
};
}

namespace {

using freej_engine::FrameAccess;
using freej_engine::FrameLease;

void require(bool condition, const char *message) {
  if (!condition)
    throw std::runtime_error(message);
}

template<class Predicate>
void wait_until(Predicate predicate, const char *message) {
  const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (!predicate()) {
    require(std::chrono::steady_clock::now() < end, message);
    std::this_thread::yield();
  }
}

class Worker : public JSyncThread {
public:
  ~Worker() override { stop(); }

  void queue(Closure *call) { deferred_calls->add_job(call); }
  void cancel() { deferred_calls->cancel_jobs(); }
  void bump() { ++calls; }

  void thread_setup() override { ++started; }
  void thread_loop() override { std::this_thread::yield(); }
  void thread_teardown() override { ++joined; }

  std::atomic<int> calls{0};
  std::atomic<int> started{0};
  std::atomic<int> joined{0};
};

void worker_retry() {
  Worker worker;
  worker.queue(NewClosure(&worker, &Worker::bump));
  require(freej_engine::WorkerAccess::fail_start(worker) == EAGAIN, "inject pthread_create failure");
  require(!worker.is_running(), "failed creation left worker running");
  worker.stop();
  require(worker.started == 0 && worker.joined == 0 && worker.calls == 0,
          "failed creation joined a worker or consumed pending jobs");

  require(worker.start() == 0, "retry failed to start");
  wait_until([&] { return worker.started == 1; }, "startup did not reach setup");
  require(worker.calls == 1 && worker.start() == EBUSY, "startup did not drain before setup");
  worker.stop();
  require(!worker.is_running() && worker.joined == 1, "successful worker failed to settle");

  // Cancel before waiting. This also exercises notification before wait(),
  // which used to leave a synchronized closure waiting indefinitely.
  std::unique_ptr<Closure> canceled(NewSyncClosure(&worker, &Worker::bump));
  worker.queue(canceled.get());
  worker.cancel();
  canceled->wait();
  require(worker.calls == 1, "cancelled closure still ran");
}

class ReusingLayer : public Layer {
public:
  ReusingLayer() {
    require(init(5, 3, 32), "initialize worker fixture");
    opened = true;
    fps.set(100);
  }

  ~ReusingLayer() override { stop(); }

  std::atomic<int> requested{0};
  std::atomic<bool> fail_feed{false};

private:
  void *feed() override {
    if (fail_feed.load())
      return nullptr;
    const auto sample = static_cast<uint32_t>(requested.load());
    bytes.fill(0xff202020 + sample);
    return bytes.data();
  }

  std::array<uint32_t, 15> bytes{};
};

FrameLease observe(Layer &layer, uint32_t pixel) {
  FrameLease frame;
  wait_until([&] {
    frame = FrameAccess::layer(layer);
    return frame && frame->pixels.front() == pixel;
  }, "worker did not publish the requested pixel");
  return frame;
}

void retain_frames() {
  SoftScreen screen;
  require(screen.init(5, 3, 32), "initialize memory screen");
  ReusingLayer layer;
  require(screen.add_layer(&layer) && layer.start() == 0, "attach and start image producer");
  const auto held = observe(layer, 0xff202020);
  layer.requested = 1;
  const auto following = observe(layer, 0xff202021);
  layer.requested = 2;
  const auto latest = observe(layer, 0xff202022);
  require(held->layout.width == 5 && held->layout.height == 3 && held->layout.stride_bytes == 20,
          "frame layout changed");
  require(held->generation < following->generation && following->generation < latest->generation &&
              held->timestamp <= latest->timestamp,
          "generation or monotonic timestamp did not advance");
  require(held->pixels.back() == 0xff202020 && following->pixels.back() == 0xff202021,
          "producer overwrote a held frame");

  screen.blit_layers();
  require(*static_cast<uint32_t *>(screen.coords(4, 2)) == 0xff202022, "blit did not use latest generation");
  layer.fail_feed = true;
  wait_until([&] { return !FrameAccess::layer(layer); }, "failed feed did not invalidate the latest frame");
  screen.rem_layer(&layer);
  require(layer.screen == nullptr && layer.list == nullptr && !layer.is_running() &&
              !FrameAccess::layer(layer) && held->pixels.back() == 0xff202020,
          "detach released a lease or kept the worker or attachment");

  layer.requested = 3;
  layer.fail_feed = false;
  require(screen.add_layer(&layer), "reattach the same caller-owned layer");
  const auto reattached = observe(layer, 0xff202023);
  require(reattached->generation > latest->generation, "reattach reused a retired generation");
  screen.reset();
  require(layer.screen == nullptr && !layer.is_running() && screen.layers.len() == 0,
          "reset did not detach and stop the producer");
}

#ifdef WITH_OGGTHEORA
class CountedEncoder : public OggTheoraEncoder {
public:
  explicit CountedEncoder(std::atomic<unsigned> &submissions) : submissions_(submissions) {}
  ~CountedEncoder() override { stop(); }

  int encode_frame() override {
    const int result = OggTheoraEncoder::encode_frame();
    ++submissions_;
    return result;
  }

private:
  std::atomic<unsigned> &submissions_;
};

void record_and_reset(const std::string &path) {
  std::atomic<unsigned> submissions{0};
  Context context;
  context.poll_events = false;
  std::unique_ptr<SoftScreen> screen(new SoftScreen);
  require(screen->init(80, 60, 32) && context.add_screen(screen.get()), "install 80x60 memory screen");
  auto *viewport = screen.release();
  auto *pixels = static_cast<uint32_t *>(viewport->get_surface());
  std::fill_n(pixels, viewport->geo.pixelsize, 0xff103050);

  std::unique_ptr<CountedEncoder> encoder(new CountedEncoder(submissions));
  require(encoder->set_filedump(path.c_str()), "open output before starting the encoder");
  require(context.add_encoder(encoder.get()), "attach Theora encoder");
  encoder.release();

  const auto held = FrameAccess::viewport(*viewport);
  require(held && held->pixels.back() == 0xff103050, "initial viewport snapshot mismatch");
  wait_until([&] { return submissions >= 2; }, "recording did not start");
  std::unique_ptr<void, decltype(&std::free)> replacement(std::calloc(96 * 64, 4), std::free);
  require(bool(replacement), "allocate resized viewport");
  std::fill_n(static_cast<uint32_t *>(replacement.get()), 96 * 64, 0xff406080);
  viewport->set_buffer(replacement.release());
  viewport->geo.init(96, 64, 32);
  context.cafudda(0.0);
  const auto resized = FrameAccess::viewport(*viewport);
  require(resized && resized->layout.width == 96 && resized->pixels.back() == 0xff406080 &&
              held->layout.width == 80 && held->pixels.back() == 0xff103050,
          "resize invalidated an outstanding viewport frame");
  wait_until([&] { return submissions >= 5; }, "recording stopped at resize");
  context.reset();
  require(context.screens.len() == 0 && context.screen == nullptr && held->pixels.back() == 0xff103050,
          "reset invalidated a consumer or retained the recording screen");

  std::ifstream file(path, std::ios::binary | std::ios::ate);
  require(file && file.tellg() > 0, "recording flush left no bytes");
  file.seekg(0);
  std::array<char, 4> signature{};
  file.read(signature.data(), signature.size());
  require(signature == std::array<char, 4>{{'O', 'g', 'g', 'S'}}, "missing Ogg header");
  std::printf("RECORD submissions %u\n", submissions.load());
}
#endif

}  // namespace

int main(int argc, char **argv) {
  if (argc > 2)
    return 2;
  const bool workers_only = argc == 2 && std::string(argv[1]) == "--workers-only";
  std::string recording;
  if (argc == 2 && !workers_only) {
    recording = argv[1];
  } else {
    char name[] = "lifetime.XXXXXX";
    const int temp = mkstemp(name);
    if (temp < 0)
      return 2;
    close(temp);
    unlink(name);
    recording = name;
  }
  int result = 0;
  try {
    find_best_memcpy();
    worker_retry();
    std::puts("PASS worker creation, retry, join, and closure cancellation");
    retain_frames();
    std::puts("PASS frame generations, active detach/reattach, failed feed and reset");
#ifdef WITH_OGGTHEORA
    if (!workers_only) {
      record_and_reset(recording);
      std::puts("PASS retained viewport generations and recording on reset");
    }
#endif
  } catch (const std::exception &error) {
    std::fprintf(stderr, "FAIL %s\n", error.what());
    result = 1;
  }
  if (argc == 1 || workers_only)
    unlink(recording.c_str());
  return result;
}
