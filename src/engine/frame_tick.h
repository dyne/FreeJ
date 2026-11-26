#ifndef FREEJ_FRAME_TICK_H
#define FREEJ_FRAME_TICK_H

namespace freej_engine {

// A host selects one screen at a time and owns its state for the tick.
// Operations that do not apply to the selected screen are host no-ops.
class FrameTick {
public:
  virtual void dispatch_input() = 0;
  virtual bool select_first_screen() = 0;
  virtual bool select_next_screen() = 0;
  virtual void clear_screen() = 0;
  virtual void resize_screen() = 0;
  virtual void blit_screen() = 0;
  virtual void present_screen() = 0;
  virtual void collect_garbage() = 0;
  virtual void sample_completion_time() = 0;

protected:
  ~FrameTick() = default;
};

// Run one tick and return control to the caller, which owns pacing.
void run_frame_tick(FrameTick &host);

}

#endif
