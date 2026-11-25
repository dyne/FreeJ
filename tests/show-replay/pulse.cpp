#include <jack/jack.h>
#include <pthread.h>
#include <signal.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>

static bool equalizer_tone = false;
static bool sparse = false;
static unsigned period_index = 0;

static int send_pulse(jack_nframes_t frames, void *output)
{
    auto *samples = static_cast<jack_default_audio_sample_t *>(
        jack_port_get_buffer(static_cast<jack_port_t *>(output), frames));
    if (equalizer_tone) {
        // Bin 24 completes three cycles per 256 samples, so the known phase
        // also repeats every 2048 samples on the second server configuration.
        for (jack_nframes_t i = 0; i < frames; ++i)
            samples[i] = 0.25 + 0.25 * std::cos(2 * std::acos(-1.) * 3 * i / 256);
    } else {
        std::memset(samples, 0, frames * sizeof(*samples));
        if (!sparse || period_index++ % 12 == 0) samples[0] = 0.5f;
    }
    return 0;
}

int main(int argc, char **argv)
{
    sparse = argc == 2 && std::strcmp(argv[1], "--sparse") == 0;
    if (argc != 1 && !sparse && (argc != 4 || std::strcmp(argv[1], "--equalizer") != 0))
        return 1;
    equalizer_tone = argc == 4;
    const unsigned rate = equalizer_tone ? std::strtoul(argv[2], nullptr, 10) : 48000;
    const unsigned period = equalizer_tone ? std::strtoul(argv[3], nullptr, 10) : 2048;
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGTERM);
    sigaddset(&signals, SIGINT);
    if (pthread_sigmask(SIG_BLOCK, &signals, nullptr))
        return 1;

    auto *client = jack_client_open("replay_pcm", JackNoStartServer, nullptr);
    if (!client)
        return 1;
    auto *output = jack_port_register(client, "pulse", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if (!output || jack_get_sample_rate(client) != rate || jack_get_buffer_size(client) != period ||
        jack_set_process_callback(client, send_pulse, output) || jack_activate(client)) {
        jack_client_close(client);
        return 1;
    }

    std::puts(jack_port_name(output));
    std::fflush(stdout);
    int signal;
    const int result = sigwait(&signals, &signal);
    jack_client_close(client);
    return result ? 1 : 0;
}
