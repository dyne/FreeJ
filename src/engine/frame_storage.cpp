#include "frame_storage.h"
#include "surface_lock.h"

#include <cstring>
#include <limits>
#include <utility>

#include <layer.h>
#include <screen.h>

namespace freej_engine {

std::shared_ptr<Frame> FrameStore::prepare(const void *pixels, FrameLayout layout) {
  constexpr std::size_t pixel_bytes = sizeof(std::uint32_t);
  if (!pixels || !layout.width || !layout.height ||
      layout.width > std::numeric_limits<std::size_t>::max() / pixel_bytes)
    return {};
  const std::size_t row = layout.width * pixel_bytes;
  if (layout.stride_bytes < row || row > std::numeric_limits<std::size_t>::max() / layout.height ||
      layout.stride_bytes > std::numeric_limits<std::size_t>::max() / layout.height)
    return {};

  auto frame = reusable_ && reusable_.unique() ? std::move(reusable_) : std::make_shared<Frame>();
  frame->layout = {layout.width, layout.height, row, layout.format};
  frame->pixels.resize(row / pixel_bytes * layout.height);
  const auto *source = static_cast<const char *>(pixels);
  auto *target = reinterpret_cast<char *>(frame->pixels.data());
  if (row == layout.stride_bytes) {
    std::memcpy(target, source, row * layout.height);
  } else {
    for (unsigned line = 0; line < layout.height; ++line)
      std::memcpy(target + line * row, source + line * layout.stride_bytes, row);
  }
  return frame;
}

void FrameStore::publish(std::shared_ptr<Frame> frame) {
  if (frame) {
    frame->generation = ++generation_;
    frame->timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch());
  }
  reusable_ = std::atomic_exchange(&current_, std::move(frame));
}

FrameLease FrameStore::acquire() const {
  return std::atomic_load(&current_);
}

void FrameStore::clear() {
  publish({});
  reusable_.reset();
}

FrameLease FrameAccess::layer(const Layer &layer) {
  return layer.frames_->acquire();
}

FrameLease FrameAccess::viewport(const ViewPort &viewport) {
  return viewport.frames_->acquire();
}

void FrameAccess::capture(ViewPort &viewport) {
  if (!viewport.encoders.len()) {
    if (viewport.capture_active_) {
      viewport.frames_->clear();
      viewport.capture_active_ = false;
    }
    return;
  }
  const SurfaceLock display(viewport);
  if (!display)
    return;

  // These viewports expose 32-bit scanlines using the geometry pitch.
  const auto pixel_format = viewport.get_pixel_format();
  const FrameFormat format = pixel_format == ViewPort::BGRA32 ? FrameFormat::bgra32 :
      pixel_format == ViewPort::ARGB32 ? FrameFormat::argb32 : FrameFormat::rgba32;
  viewport.frames_->publish(viewport.frames_->prepare(viewport.get_surface(),
      {viewport.geo.w, viewport.geo.h, viewport.geo.bytewidth, format}));
  viewport.capture_active_ = true;
}

}
