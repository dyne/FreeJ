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
    m_audio = new AudioCollector("metro:120_bpm", 1024, 48000); //
    //m_audio = new AudioCollector("", 1024, 48000);
//    m_audio = new AudioCollector("MPlayer [13728]:out_0", 1024, 48000);
//    m_audio = new AudioCollector("system:capture_1", 1024, 48000);

    m_freej->screen->add_audio(m_audio->Jack); //

    if (m_audio) //
    { //
        m_enc = new OggTheoraEncoder();
        if (m_enc)
        {
            m_enc->video_quality = 10;
            m_enc->video_bitrate = 100000;
            m_enc->audio_quality = 1;
            m_enc->audio_bitrate = 48000;

            m_enc->use_audio = true; //
//             m_enc->use_audio = false;
            m_enc->audio = m_audio; //



// 	    if(shout_set_host(m_enc->ice, "skimeuzac.com"))
// 			qDebug() << "shout_set_host: " << shout_get_error(m_enc->ice);

	    if(shout_set_host(m_enc->ice, "localhost"))
			qDebug() << "shout_set_host: " << shout_get_error(m_enc->ice);

	    if(shout_set_port(m_enc->ice, 8000))
			qDebug() << "shout_set_port: " << shout_get_error(m_enc->ice);

	    if(shout_set_name(m_enc->ice, "qfreej streaming test"))
			qDebug() << "shout_set_title: " << shout_get_error(m_enc->ice);

	    if(shout_set_user(m_enc->ice, "source"))
			qDebug() << "shout_set_user: " << shout_get_error(m_enc->ice);

	    if(shout_set_password(m_enc->ice, "test!"))
			qDebug() << "shout_set_pass: " << shout_get_error(m_enc->ice);

	    if(shout_set_mount(m_enc->ice, "freejcpp.ogg"))
			qDebug() << "shout_set_mount: " << shout_get_error(m_enc->ice);

            m_freej->add_encoder(m_enc);

	    if( shout_open(m_enc->ice) == SHOUTERR_SUCCESS ) {

		qDebug() << "streaming on url: http://" << shout_get_host(m_enc->ice) << ":" \
			 << shout_get_port(m_enc->ice) <<  shout_get_mount(m_enc->ice);

		m_enc->write_to_stream = true;
	    } else {

		qDebug() << "error connecting to server " << shout_get_host(m_enc->ice) << ":" \
				<< shout_get_error(m_enc->ice);

		m_enc->write_to_stream = false;
	    }


	    m_enc->set_filedump("Video/dump.ogg");
            qDebug() << "fps :" << m_freej->fps.get();
        }
    } //
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
