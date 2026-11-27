#include <sdl_controller.h>
#include <vector>

SdlController::SdlController() 
    : Controller()
{
}

SdlController::~SdlController()
{
}

void SdlController::poll_sdlevents(Uint32 first, Uint32 last) {
  const int pending = SDL_PeepEvents(nullptr, 0, SDL_PEEKEVENT, first, last);
  std::vector<SDL_Event> unhandled;
  for (int index = 0; index < pending; ++index) {
    if (SDL_PeepEvents(&event, 1, SDL_GETEVENT, first, last) != 1)
      break;
    if (!dispatch())
      unhandled.push_back(event);
  }
  if (!unhandled.empty())
    SDL_PeepEvents(unhandled.data(), unhandled.size(), SDL_ADDEVENT, first, last);
}
