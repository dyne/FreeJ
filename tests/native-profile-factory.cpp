#include <context.h>
#include <jack/jack.h>
#include <cstdio>
#include <cstring>
#include <memory>

int main(int argc, char **argv)
{
    Context context;
    if (!context.init())
        return 1;

    std::unique_ptr<ViewPort> screen(Factory<ViewPort>::new_instance("Screen", "soft"));
    std::unique_ptr<Layer> image(Factory<Layer>::new_instance("ImageLayer", "sdl"));
    if (!screen || !image || !screen->init(80, 60, 32) || !screen->get_surface())
        return 2;

    std::puts("Installed factory created Screen/soft and ImageLayer/sdl");
    if (argc == 2 && std::strcmp(argv[1], "--jack") == 0) {
        jack_client_t *client = jack_client_open("freej_profile_probe", JackNoStartServer, NULL);
        if (!client)
            return 3;
        const bool expected = jack_get_sample_rate(client) == 48000
            && jack_get_buffer_size(client) == 2048;
        jack_client_close(client);
        if (!expected)
            return 4;
        std::puts("Installed JACK client connected at 48000 Hz / 2048 frames");
    }
    return 0;
}
