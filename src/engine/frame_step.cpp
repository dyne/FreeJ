#include "frame_step.h"
#include "frame_tick.h"

#include <context.h>
#include <jsparser.h>

namespace freej_engine {
namespace {

class ContextFrameTick final : public FrameTick {
public:
  ContextFrameTick(Context &context, const FrameHost &host)
    : context_(context), host_(host) {}

  void dispatch_input() override {
    if (host_.dispatch_input)
      host_.dispatch_input();
  }

  bool select_first_screen() override {
    screen_ = context_.screens.begin();
    return screen_ != nullptr;
  }

  bool select_next_screen() override {
    screen_ = static_cast<ViewPort *>(screen_->next);
    return screen_ != nullptr;
  }

  void clear_screen() override {
    if (context_.clear_all)
      screen_->clear();
  }

  void resize_screen() override {
    if (screen_->changeres)
      screen_->handle_resize();
  }

  void blit_screen() override { screen_->blit_layers(); }
  void present_screen() override { host_.present(*screen_); }

  void collect_garbage() override {
    if (context_.js)
      context_.js->gc();
  }

  void sample_completion_time() override { completed = host_.now(); }

  timeval completed{};

private:
  Context &context_;
  const FrameHost &host_;
  ViewPort *screen_ = nullptr;
};

}

timeval step_frame(Context &context, const FrameHost &host) {
  ContextFrameTick tick(context, host);
  run_frame_tick(tick);
  return tick.completed;
}

}
