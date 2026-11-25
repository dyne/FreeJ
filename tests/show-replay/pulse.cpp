#include <jack/jack.h>
#include <pthread.h>
#include <signal.h>
#include <cstdio>
#include <cstring>

static int send_pulse(jack_nframes_t frames, void *output)
{
    auto *samples = static_cast<jack_default_audio_sample_t *>(
        jack_port_get_buffer(static_cast<jack_port_t *>(output), frames));
    std::memset(samples, 0, frames * sizeof(*samples));
    samples[0] = 0.5f;
    return 0;
}

int main()
{
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
    if (!output || jack_get_sample_rate(client) != 48000 || jack_get_buffer_size(client) != 2048 ||
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
