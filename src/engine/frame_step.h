#ifndef FREEJ_FRAME_STEP_H
#define FREEJ_FRAME_STEP_H

#include <functional>
#include <sys/time.h>

class Context;
class ViewPort;

namespace freej_engine {

struct FrameHost {
  std::function<void()> dispatch_input; // Optional when the host has input to dispatch.
  std::function<void(ViewPort &)> present;
  std::function<timeval()> now;
};

// Execute one tick and sample the host clock at completion. The host owns
// event pumping and pacing. Layer workers keep their own feed/filter schedule.
timeval step_frame(Context &context, const FrameHost &host);

}

#endif
