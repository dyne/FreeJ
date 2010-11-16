#include <Sound.h>
#include <fps.h>

Sound::Sound(Context *freej)
{
    m_freej = freej;
    m_audio = NULL;
    m_enc = NULL;

    m_audio = new AudioCollector("metro:120_bpm", 1024, 48000);
    if (m_audio)
    {
        m_enc = new OggTheoraEncoder();
        if (m_enc)
        {
            m_enc->video_quality = 50;
            m_enc->video_bitrate = 1000000;
            m_enc->audio_quality = 6;

            VideoEncoder *enc = (VideoEncoder*)(m_enc);
            enc->use_audio = true;
            enc->audio = m_audio;

            //enc->active = true;           //me

            //FPS *fps = new FPS();
            //fps->init(12);
            //fps->set(12);
            //enc->fps = fps;
            m_freej->add_encoder(enc);
            enc->set_filedump("Video/dump.ogg");
        }
    }

    //    AudioLayer *lay = new AudioLayer();
    //    if(lay)
    //    {
    //        lay->open("capture_1:");
    //        qDebug() << "l'audio devrait être ouvert";
    //    }
}

Sound::~Sound()
{
    if (m_audio)
        delete m_audio;
    if (m_enc)
        delete m_enc;
}
