#include "frame_tick.h"

namespace freej_engine {

void run_frame_tick(FrameTick &host) {
  host.dispatch_input();
  for (bool selected = host.select_first_screen(); selected; selected = host.select_next_screen()) {
    host.clear_screen();
    host.resize_screen();
    host.blit_screen();
    host.present_screen();
  }
  host.collect_garbage();
  host.sample_completion_time();
}

}
