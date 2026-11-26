#include <context.h>
#include <audio_collector.h>
#include <audio_pipewire.h>
#include <chrono>
#include <thread>
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
    if (argc == 2 && std::strcmp(argv[1], "--pipewire") == 0) {
        char source[] = "replay_pcm:pulse";
        AudioCollector audio(source, 2048, 48000);
        if (!audio.IsConnected())
            return 3;
        const auto stop = std::chrono::steady_clock::now() + std::chrono::seconds(4);
        while (std::chrono::steady_clock::now() < stop) {
            audio.GetFFT();
            AudioCapture::Position position;
            if (audio.CapturePosition(position) && position.rate == 48000 &&
                position.period == 2048 && audio.GetAudioBuffer()[0] == 0.5f) {
                std::puts("Installed PipeWire client captured at 48000 Hz / 2048 frames");
                return 0;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return 4;
    }
    return 0;
}
