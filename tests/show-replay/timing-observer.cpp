// Linux reference timing observer. It leaves the scheduler and routing intact.
#include "context.h"
#include "osc_ctrl.h"
#include <audio_collector.h>

#include "pipewire-observer.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <time.h>
#include <cstdio>
#include <cstring>

static int trace = -1;
static pw_filter_events audio_events{};
static void (*process_audio)(void *, spa_io_position *) = nullptr;
static void *audio_argument = nullptr;
static bool pulse_source = false;
static bool visible_audio = false;
static const float *source_output = nullptr;
static uint64_t sample_position, previous_xrun;

__attribute__((constructor)) static void open_trace()
{
    const char *path = std::getenv("FREEJ_TIMING_TRACE");
    if (path) {
        trace = open(path, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0600);
        if (trace < 0)
            std::abort();
    }
    pulse_source = std::getenv("FREEJ_TIMING_PULSE_SOURCE") != nullptr;
    visible_audio = std::getenv("FREEJ_TIMING_VISIBLE_AUDIO") != nullptr;
}

template<class Function>
static Function next_symbol(const char *symbol)
{
    auto function = reinterpret_cast<Function>(dlsym(RTLD_NEXT, symbol));
    if (!function)
        std::abort();
    return function;
}

static long long monotonic_ns()
{
    timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now))
        std::abort();
    return static_cast<long long>(now.tv_sec) * 1000000000 + now.tv_nsec;
}

static void event(const char *kind, long long value, long long detail = 0, long long at = 0)
{
    if (trace < 0)
        return;
    char line[96];
    const int n = std::snprintf(line, sizeof(line), "%lld %s %lld %lld\n",
                                at ? at : monotonic_ns(), kind, value, detail);
    if (n < 0 || n >= static_cast<int>(sizeof(line)) || write(trace, line, n) != n)
        std::abort();
}

extern "C" bool SDLCALL SDL_UpdateWindowSurface(SDL_Window *window)
{
    static const auto update = next_symbol<decltype(&SDL_UpdateWindowSurface)>("SDL_UpdateWindowSurface");
    int marker = -1;
    if (visible_audio) {
        auto *surface = SDL_GetWindowSurface(window);
        Uint8 r, g, b;
        if (!surface || surface->w != 80 || surface->h != 60 ||
            !SDL_ReadSurfacePixel(surface, 20, 16, &r, &g, &b, nullptr))
            std::abort();
        marker = (r << 16) | (g << 8) | b;
    }
    const auto submitted = monotonic_ns();
    const bool result = update(window);
    event("present", result ? 0 : -1, visible_audio ? marker : 0, submitted);
    return result;
}

int osc_command_handler(const char *path, const char *types, lo_arg **args,
                        int argc, lo_message message, void *controller)
{
    static const auto handler = next_symbol<decltype(&osc_command_handler)>(
        "_Z19osc_command_handlerPKcS0_PP6lo_argiP11lo_message_Pv");
    if (std::strncmp(path, "/timing/", 8) == 0)
        event("osc-receive", std::atoi(path + 8), argc);
    return handler(path, types, args, argc, message, controller);
}

int Controller::JSCall(const char *name, int argc, jsval *args)
{
    using Call = int (*)(Controller *, const char *, int, jsval *);
    static const auto call = next_symbol<Call>("_ZN10Controller6JSCallEPKciPl");
    if (std::strncmp(name, "timing", 6) != 0)
        return call(this, name, argc, args);
    const auto dispatched = monotonic_ns();
    const int callbacks = call(this, name, argc, args);
    event("osc-dispatch", std::atoi(name + 6), callbacks, dispatched);
    return callbacks;
}

static void observe_process(void *, spa_io_position *position)
{
    const auto &clock = position->clock;
    sample_position = clock.position;
    source_output = nullptr;
    const auto begin = monotonic_ns();
    process_audio(audio_argument, position);
    const auto end = monotonic_ns();
    if (clock.xrun > previous_xrun)
        event("pipewire-xrun", clock.xrun - previous_xrun);
    previous_xrun = clock.xrun;
    if (!pulse_source)
        event("process-duration", clock.duration, end - begin, end);
    if (pulse_source && source_output && (!visible_audio || source_output[0] == 0.5f))
        event("pulse-source", sample_position, clock.duration);
}

extern "C" void pw_filter_add_listener(pw_filter *filter, spa_hook *hook,
                                        const pw_filter_events *events, void *data)
{
    static const auto add = next_symbol<decltype(&pw_filter_add_listener)>("pw_filter_add_listener");
    if (trace >= 0 && events && events->process) {
        process_audio = events->process;
        audio_argument = data;
        audio_events = *events;
        audio_events.process = observe_process;
        events = &audio_events;
        event("xrun-registration", 0);
    }
    add(filter, hook, events, data);
}

extern "C" void *pw_filter_get_dsp_buffer(void *port, uint32_t frames)
{
    static const auto get = next_symbol<decltype(&pw_filter_get_dsp_buffer)>("pw_filter_get_dsp_buffer");
    auto *buffer = static_cast<float *>(get(port, frames));
    if (pulse_source)
        source_output = buffer;
    if (trace >= 0 && !pulse_source && frames && buffer &&
        port == observed_input.load(std::memory_order_acquire)) {
        if (visible_audio)
            event("input-period", sample_position, frames);
        if (buffer[0] == 0.5f)
            event("pulse-capture", sample_position, frames);
    }
    return buffer;
}

float *AudioCollector::GetFFT()
{
    using Get = float *(*)(AudioCollector *);
    static auto get = next_symbol<Get>("_ZN14AudioCollector6GetFFTEv");
    float *bands = get(this);
    if (visible_audio) {
        AudioCapture::Position position;
        if (CapturePosition(position))
            event("fft-window", position.first, bands[15] > 5);
    }
    return bands;
}
