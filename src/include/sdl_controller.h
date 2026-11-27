#ifndef __SDL_CONTROLLER_H__
#define __SDL_CONTROLLER_H__

#include <controller.h>
#include <SDL3/SDL.h>

class SdlController: public Controller {
  public:
    SDL_Event event; ///< SDL event structure

    SdlController();
    ~SdlController();
    void poll_sdlevents(Uint32 first, Uint32 last);
    ///< helper function to filter and redispatch unhandled SDL_Events
    ///< calls dispatch() foreach event in the selected range
 
};

#endif
