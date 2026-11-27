#ifndef FREEJ_AUDIO_CAPTURE_H
#define FREEJ_AUDIO_CAPTURE_H

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <vector>

// One device producer and one analysis consumer. Front and back are private to
// their respective threads. An exchange gives exclusive ownership of the middle
// slot to its caller. A slow reader skips obsolete complete windows.
class AudioCapture {
public:
  struct Position {
    uint32_t first = 0, rate = 0, period = 0, generation = 0;
  };

  explicit AudioCapture(size_t length) {
    if (!length) throw std::invalid_argument("capture window must be positive");
    for (auto &slot : slots_) slot.samples.resize(length);
  }
  ~AudioCapture() {
    if (wake_pipe_[0] >= 0) { close(wake_pipe_[0]); close(wake_pipe_[1]); }
  }

  // Configure on the control thread before registering the port. One
  // nonblocking byte wakes the FFT worker when an empty mailbox becomes
  // ready. Further publications coalesce until the worker takes a slot.
  bool enable_wakeup() {
    if (wake_pipe_[0] >= 0) return true;
    int ends[2];
    if (pipe(ends)) return false;
    if (fcntl(ends[0], F_SETFD, FD_CLOEXEC) < 0 || fcntl(ends[1], F_SETFD, FD_CLOEXEC) < 0 ||
        fcntl(ends[1], F_SETFL, O_NONBLOCK) < 0) {
      close(ends[0]); close(ends[1]); return false;
    }
    wake_pipe_[0] = ends[0]; wake_pipe_[1] = ends[1];
    return true;
  }
  void wake() const {
    if (wake_pipe_[1] < 0) return;
    const char byte = 1;
    ssize_t n;
    do { n = ::write(wake_pipe_[1], &byte, 1); } while (n < 0 && errno == EINTR);
    // EAGAIN means a wake is already pending.
  }
  bool wait() const {
    char bytes[32];
    ssize_t n;
    do { n = ::read(wake_pipe_[0], bytes, sizeof(bytes)); } while (n < 0 && errno == EINTR);
    return n > 0;
  }

  void write(const float *input, uint32_t frames, uint32_t first, uint32_t rate) {
    if (active_ && (first != next_ || rate != rate_ || frames != period_))
      gap();
    active_ = true;
    next_ = first + frames; // Frame time rolls over modulo 2^32.
    rate_ = rate;
    period_ = frames;
    for (size_t offset = 0; offset < frames;) {
      auto &slot = slots_[back_];
      if (!used_) {
        slot.position.first = first + offset;
        slot.position.rate = rate;
        slot.position.period = frames;
        slot.position.generation = generation_.load(std::memory_order_relaxed);
      }
      const auto n = std::min(frames - offset, slot.samples.size() - used_);
      std::copy_n(input + offset, n, slot.samples.data() + used_);
      offset += n;
      used_ += n;
      if (used_ == slot.samples.size()) {
        const auto previous = middle_.exchange(back_ | ready, std::memory_order_acq_rel);
        back_ = previous & ~ready;
        used_ = 0;
        if (!(previous & ready)) wake();
      }
    }
  }

  // Only the device producer calls gap while active. The owner also calls it
  // after the device has stopped the producer.
  void gap() {
    if (active_) generation_.fetch_add(1, std::memory_order_release);
    active_ = false;
    used_ = 0;
  }

  uint32_t generation() const { return generation_.load(std::memory_order_acquire); }

  bool read(float *output, Position &position) {
    if (!(middle_.load(std::memory_order_acquire) & ready)) return false;
    front_ = middle_.exchange(front_, std::memory_order_acq_rel) & ~ready;
    const auto &slot = slots_[front_];
    position = slot.position;
    std::copy(slot.samples.begin(), slot.samples.end(), output);
    return true;
  }

private:
  static_assert(ATOMIC_INT_LOCK_FREE == 2, "capture publication must be lock-free");
  static const unsigned ready = 4;
  struct Slot { Position position; std::vector<float> samples; };
  std::array<Slot, 3> slots_;
  int wake_pipe_[2] = {-1, -1};
  unsigned front_ = 0, back_ = 2;
  std::atomic<unsigned> middle_{1}, generation_{0};
  size_t used_ = 0;
  bool active_ = false;
  uint32_t next_ = 0, rate_ = 0, period_ = 0;
};

#endif
