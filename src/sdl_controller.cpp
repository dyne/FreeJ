#include <sdl_controller.h>
#include <SDL2/SDL.h>

SdlController::SdlController() 
    : Controller()
{
}

SdlController::~SdlController()
{
}

void SdlController::poll_sdlevents(Uint32 eventmask) {
	int res;
	SDL_Event user_event;

	res = SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, eventmask);
	if (!res) return;

	user_event.type=SDL_USEREVENT;
	user_event.user.code=42;
	SDL_PeepEvents(&user_event, 1, SDL_ADDEVENT, SDL_ALLEVENTS);

	res = SDL_PeepEvents(&event, 1, SDL_GETEVENT, eventmask|SDL_EVENTMASK(SDL_USEREVENT));
	while (res>0) {
		int handled = dispatch(); // <<< virtual
		if (handled == 0)
				SDL_PeepEvents(&event, 1, SDL_ADDEVENT, SDL_ALLEVENTS);
		res = SDL_PeepEvents(&event, 1, SDL_GETEVENT, eventmask|SDL_EVENTMASK(SDL_USEREVENT));
		if (event.type == SDL_USEREVENT)
				res = 0;
	}
	//return 1;
}


