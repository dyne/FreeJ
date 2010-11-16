#ifndef SOUND_H
#define SOUND_H
#include <audio_layer.h>
#include <audio_jack.h>
#include <audio_collector.h>
#include <audio_input.h>
#include <oggtheora_encoder.h>
#include <QObject>

class Sound
{
public:
    Sound(Context *);
    ~Sound();

private:
    Context *m_freej;
    AudioCollector *m_audio;
    OggTheoraEncoder *m_enc;
};

#endif // SOUND_H
