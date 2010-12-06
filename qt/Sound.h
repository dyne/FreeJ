#ifndef SOUND_H
#define SOUND_H
#include <audio_layer.h>
#include <audio_jack.h>
#include <audio_collector.h>
#include <audio_input.h>
#include <oggtheora_encoder.h>
#include <QPushButton>

class Sound : public QWidget
{
    Q_OBJECT
public:
    Sound(Context *, QWidget *parent = 0);
    ~Sound();

public slots:
    void sound();

private:
    Context *m_freej;
    AudioCollector *m_audio;
    OggTheoraEncoder *m_enc;
};

#endif // SOUND_H
