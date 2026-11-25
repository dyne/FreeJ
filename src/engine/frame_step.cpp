#include "frame_step.h"

#include <context.h>
#include <jsparser.h>

namespace freej_engine {

timeval step_frame(Context &context, const FrameHost &host) {
  if (host.dispatch_input)
    host.dispatch_input();

  for (ViewPort *screen = context.screens.begin(); screen;
       screen = static_cast<ViewPort *>(screen->next)) {
    if (context.clear_all)
      screen->clear();
    if (screen->changeres)
      screen->handle_resize();
    screen->blit_layers();
    host.present(*screen);
  }

  if (context.js)
    context.js->gc();
  return host.now();
}

}
