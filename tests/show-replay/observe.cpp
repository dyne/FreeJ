#include "context.h"

#include <dlfcn.h>
#include "pipewire-observer.h"
#include <cstdio>
#include <cstring>
#include <vector>

static bool observing(const char *kind)
{
    const char *mode = std::getenv("FREEJ_REPLAY_PROBE");
    return mode && std::strcmp(mode, kind) == 0;
}

extern "C" bool SDLCALL SDL_UpdateWindowSurface(SDL_Window *window)
{
    using Update = decltype(&SDL_UpdateWindowSurface);
    static const auto update = reinterpret_cast<Update>(dlsym(RTLD_NEXT, "SDL_UpdateWindowSurface"));
    if (!update)
        std::abort();
    if (!observing("video"))
        return update(window);

    auto *surface = SDL_GetWindowSurface(window);
    if (!surface)
        std::abort();
    std::vector<Uint8> rgb(surface->w * surface->h * 3);
    for (int y = 0; y < surface->h; ++y) {
        for (int x = 0; x < surface->w; ++x) {
            auto *out = &rgb[(y * surface->w + x) * 3];
            if (!SDL_ReadSurfacePixel(surface, x, y, out, out + 1, out + 2, nullptr))
                std::abort();
        }
    }

    const bool result = update(window);
    if (result) {
        static auto *frames = std::fopen("frames.rgb", "wb");
        static auto *states = std::fopen("presentations.txt", "w");
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

extern "C" void *pw_filter_get_dsp_buffer(void *port, uint32_t frames)
{
    using GetBuffer = decltype(&pw_filter_get_dsp_buffer);
    static const auto get_buffer =
        reinterpret_cast<GetBuffer>(dlsym(RTLD_NEXT, "pw_filter_get_dsp_buffer"));
    if (!get_buffer)
        std::abort();
    void *buffer = get_buffer(port, frames);
    static bool seen = false;
    if (!seen && observing("audio") && buffer && frames == 2048 &&
        port == observed_input.load(std::memory_order_acquire)) {
        const auto *samples = static_cast<const float *>(buffer);
        if (samples[0] == 0.5f) {
            uint32_t nonzero = 1;
            for (uint32_t i = 1; i < frames; ++i)
                nonzero += (samples[i] != 0);
            // Record one real capture block. This observer is used only for
            // the historical reproducer, outside any timing measurement.
            if (auto *capture = std::fopen("capture.txt", "w")) {
                std::fprintf(capture, "freej:input %u %.3f %u\n",
                             frames, samples[0], nonzero);
                std::fclose(capture);
            }
            seen = true;
        }
    }
    return buffer;
}
