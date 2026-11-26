#include <pipewire/pipewire.h>
#include <pipewire/filter.h>
#include <pthread.h>
#include <signal.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <unistd.h>

struct Source {
    void *port = nullptr;
    bool tone = false, sparse = false;
    unsigned period_index = 0, rate = 48000, period = 2048;
};

static void send_pulse(void *data, spa_io_position *position)
{
    auto &source = *static_cast<Source *>(data);
    const unsigned frames = position->clock.duration;
    auto *samples = static_cast<float *>(pw_filter_get_dsp_buffer(source.port, frames));
    if (!samples)
        return;
    if (position->clock.rate.denom != source.rate * position->clock.rate.num || frames != source.period)
        std::abort();
    if (source.tone) {
        for (unsigned i = 0; i < frames; ++i)
            samples[i] = 0.25 + 0.25 * std::cos(2 * std::acos(-1.) * 3 * i / 256);
    } else {
        std::memset(samples, 0, frames * sizeof(*samples));
        if (!source.sparse || source.period_index++ % 12 == 0) samples[0] = 0.5f;
    }
}

int main(int argc, char **argv)
{
    Source source;
    source.sparse = argc == 2 && std::strcmp(argv[1], "--sparse") == 0;
    source.tone = argc == 4 && std::strcmp(argv[1], "--equalizer") == 0;
    if (argc != 1 && !source.sparse && !source.tone)
        return 1;
    if (source.tone) {
        source.rate = std::strtoul(argv[2], nullptr, 10);
        source.period = std::strtoul(argv[3], nullptr, 10);
    }
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGTERM);
    sigaddset(&signals, SIGINT);
    if (pthread_sigmask(SIG_BLOCK, &signals, nullptr))
        return 1;

    pw_init(nullptr, nullptr);
    auto *loop = pw_thread_loop_new("replay-pcm", nullptr);
    if (!loop) return 1;
    auto *context = pw_context_new(pw_thread_loop_get_loop(loop), nullptr, 0);
    if (!context) return 1;
    auto *core = pw_context_connect(context, nullptr, 0);
    if (!core) return 1;
    auto *filter = pw_filter_new(core, "replay_pcm",
        pw_properties_new(PW_KEY_NODE_NAME, "replay_pcm", PW_KEY_MEDIA_TYPE, "Audio", nullptr));
    if (!filter) return 1;
    source.port = pw_filter_add_port(filter, PW_DIRECTION_OUTPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
        1, pw_properties_new(PW_KEY_PORT_NAME, "pulse", PW_KEY_FORMAT_DSP,
                             "32 bit float mono audio", nullptr), nullptr, 0);
    if (!source.port) return 1;
    pw_filter_events events{};
    events.version = PW_VERSION_FILTER_EVENTS;
    events.process = send_pulse;
    spa_hook hook{};
    pw_filter_add_listener(filter, &hook, &events, &source);
    if (pw_filter_connect(filter, PW_FILTER_FLAG_RT_PROCESS, nullptr, 0) < 0 ||
        pw_thread_loop_start(loop) < 0)
        return 1;
    bool ready = false;
    for (int attempt = 0; attempt < 100 && !ready; ++attempt) {
        pw_thread_loop_lock(loop);
        ready = pw_filter_get_node_id(filter) != SPA_ID_INVALID;
        pw_thread_loop_unlock(loop);
        if (!ready) usleep(20000);
    }

    int result = 1;
    if (ready) {
        std::puts("replay_pcm:pulse");
        std::fflush(stdout);
        int signal;
        result = sigwait(&signals, &signal);
    }
    pw_thread_loop_stop(loop);
    pw_filter_destroy(filter);
    pw_core_disconnect(core);
    pw_context_destroy(context);
    pw_thread_loop_destroy(loop);
    return result ? 1 : 0;
}
