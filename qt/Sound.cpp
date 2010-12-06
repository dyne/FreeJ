#include <Sound.h>
#include <fps.h>
#include <QFileDialog>
#include <QDebug>

Sound::Sound(Context *freej, QWidget *parent) : QWidget(parent)
{
/*
    QPushButton *button = new QPushButton ("sound", this);
    connect (button, SIGNAL(clicked()), this, SLOT(sound()));
    this->show();
*/
    m_freej = freej;
    m_audio = NULL;
    m_enc = NULL;

    // le jack output port que l'on veut enregistrer
    m_audio = new AudioCollector("metro:120_bpm", 1024, 48000);
//    m_audio = new AudioCollector("MPlayer [13728]:out_0", 1024, 48000);
//    m_audio = new AudioCollector("system:capture_1", 1024, 48000);
    //m_audio = new AudioCollector("freej:Out0", 1024, 48000);

    m_freej->screen->add_audio(m_audio->Jack);

    if (m_audio)
    {
        m_enc = new OggTheoraEncoder();
        if (m_enc)
        {
            m_enc->video_quality = 14;
            m_enc->video_bitrate = 1000000;
            m_enc->audio_quality = 6;

            m_enc->use_audio = true;
            m_enc->audio = m_audio;


            m_freej->add_encoder(m_enc);
            m_enc->set_filedump("Video/dump.ogg");
            qDebug() << "fps :" << m_freej->fps.get();
        }
    }
}

Sound::~Sound()
{
    if (m_audio)
        delete m_audio;
    if (m_enc)
        delete m_enc;
}

void Sound::sound()
{
    if (m_enc->use_audio)
    {
        qDebug() << "without sound";
        m_enc->use_audio = false;
    }
    else
    {
        qDebug() << "with sound";
        m_enc->use_audio = true;
    }
}
