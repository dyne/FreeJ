#include "context.h"

#include <dlfcn.h>
#include <jack/jack.h>
#include <cstdio>
#include <cstring>
#include <vector>

static bool observing(const char *kind)
{
    const char *mode = std::getenv("FREEJ_REPLAY_PROBE");
    return mode && std::strcmp(mode, kind) == 0;
}

extern "C" int SDL_Flip(SDL_Surface *surface)
{
    using Flip = int (*)(SDL_Surface *);
    static const auto flip = reinterpret_cast<Flip>(dlsym(RTLD_NEXT, "SDL_Flip"));
    if (!flip)
        std::abort();
    if (!observing("video"))
        return flip(surface);

    // SdlScreen::show already owns the surface lock. SDL may recycle the
    // backing storage during Flip, so read it before handing it off.
    if (surface->format->BytesPerPixel != sizeof(Uint32))
        std::abort();
    std::vector<Uint8> rgb(surface->w * surface->h * 3);
    for (int y = 0; y < surface->h; ++y) {
        const auto *row = static_cast<const Uint8 *>(surface->pixels) + y * surface->pitch;
        for (int x = 0; x < surface->w; ++x) {
            Uint32 pixel;
            std::memcpy(&pixel, row + x * sizeof(pixel), sizeof(pixel));
            auto *out = &rgb[(y * surface->w + x) * 3];
            SDL_GetRGB(pixel, surface->format, out, out + 1, out + 2);
        }
    }

    const int result = flip(surface);
    if (result == 0) {
        static auto *frames = std::fopen("frames.rgb", "wb");
        static auto *states = std::fopen("presentations.txt", "w");
        // JACK 1 invokes jackd via a child shell which also inherits this
        // preload. Resolve FreeJ state only when the FreeJ presentation runs.
        static auto **context = static_cast<Context **>(dlsym(RTLD_DEFAULT, "global_environment"));
        if (!context || !*context)
            std::abort();
        auto *screen = (*context)->screens.selected();
        auto *layer = screen->layers.selected();
        if (!frames || !states || std::fwrite(rgb.data(), 1, rgb.size(), frames) != rgb.size() ||
            std::fprintf(states, "%d %d %s\n", surface->w, surface->h,
                         layer ? layer->name : "-") < 0)
            std::abort();
    }
    return result;
}

extern "C" void *jack_port_get_buffer(jack_port_t *port, jack_nframes_t frames)
{
    using GetBuffer = void *(*)(jack_port_t *, jack_nframes_t);
    static const auto get_buffer =
        reinterpret_cast<GetBuffer>(dlsym(RTLD_NEXT, "jack_port_get_buffer"));
    if (!get_buffer)
        std::abort();
    void *buffer = get_buffer(port, frames);
    static bool seen = false;
    if (!seen && observing("audio") && buffer && frames == 2048 &&
        (jack_port_flags(port) & JackPortIsInput)) {
        const auto *samples = static_cast<const jack_default_audio_sample_t *>(buffer);
        if (samples[0] == 0.5f) {
            jack_nframes_t nonzero = 1;
            for (jack_nframes_t i = 1; i < frames; ++i)
                nonzero += (samples[i] != 0);
            // Record one real capture block. This observer is used only for
            // the historical reproducer, outside any timing measurement.
            if (auto *capture = std::fopen("capture.txt", "w")) {
                std::fprintf(capture, "%s %u %.3f %u\n", jack_port_name(port),
                             frames, samples[0], nonzero);
                std::fclose(capture);
            }
            seen = true;
        }
    }
    return buffer;
}
