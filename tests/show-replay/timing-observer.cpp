// Linux reference timing observer. It leaves the scheduler and routing intact.
#include "context.h"
#include "osc_ctrl.h"

#include <jack/jack.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <time.h>
#include <cstdio>
#include <cstring>

static int trace = -1;
static jack_client_t *jack_client = nullptr;
static JackProcessCallback process_audio = nullptr;
static void *audio_argument = nullptr;
static bool pulse_source = false;

__attribute__((constructor)) static void open_trace()
{
    const char *path = std::getenv("FREEJ_TIMING_TRACE");
    if (path) {
        trace = open(path, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0600);
        if (trace < 0)
            std::abort();
    }
    pulse_source = std::getenv("FREEJ_TIMING_PULSE_SOURCE") != nullptr;
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

extern "C" int SDL_Flip(SDL_Surface *surface)
{
    static const auto flip = next_symbol<decltype(&SDL_Flip)>("SDL_Flip");
    const auto submitted = monotonic_ns();
    const int result = flip(surface);
    event("present", result, 0, submitted);
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

static int xrun(void *)
{
    event("jack-xrun", 0);
    return 0;
}

static int jack_process(jack_nframes_t frames, void *)
{
    const int result = process_audio(frames, audio_argument);
    if (pulse_source && result == 0)
        event("pulse-source", jack_last_frame_time(jack_client), frames);
    return result;
}

extern "C" int jack_set_process_callback(jack_client_t *client, JackProcessCallback process, void *argument)
{
    static const auto set = next_symbol<decltype(&jack_set_process_callback)>("jack_set_process_callback");
    if (trace < 0)
        return set(client, process, argument);
    jack_client = client;
    process_audio = process;
    audio_argument = argument;
    const int xrun_registration = jack_set_xrun_callback(client, xrun, nullptr);
    event("xrun-registration", xrun_registration);
    return set(client, jack_process, nullptr);
}

extern "C" void *jack_port_get_buffer(jack_port_t *port, jack_nframes_t frames)
{
    static const auto get = next_symbol<decltype(&jack_port_get_buffer)>("jack_port_get_buffer");
    void *buffer = get(port, frames);
    if (trace >= 0 && jack_client && !pulse_source && frames && buffer &&
        (jack_port_flags(port) & JackPortIsInput) &&
        static_cast<const jack_default_audio_sample_t *>(buffer)[0] == 0.5f) {
        event("pulse-capture", jack_last_frame_time(jack_client), frames);
    }
    return buffer;
}
