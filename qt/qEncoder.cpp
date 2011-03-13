#include <qEncoder.h>
#include <QDebug>

QEncoder::QEncoder(Qfreej *qfreej) : QWidget()
{
  setAttribute(Qt::WA_DeleteOnClose);
  setAttribute(Qt::WA_QuitOnClose, false);
  m_enc = NULL;
  m_Qjack = NULL;
  m_streaming = false;
  m_Qfreej = qfreej;
  m_freej = qfreej->getFreej();
  init();
  show();
}

QEncoder::~QEncoder()
{
  if (m_enc)
  {
    m_enc->stop();
    m_enc->active = false;
    delete m_enc;
    m_enc = NULL;
  }
  if (m_Qjack)
    (m_Qjack->getJack())->isEncoded(false);
  m_Qfreej->resetEnc();
}

void QEncoder::init()
{
  QGridLayout *layoutG = new QGridLayout;
  
  m_streamButton = new QPushButton("Stream START", this);
  connect (m_streamButton, SIGNAL(clicked()), this, SLOT(stream()));
  layoutG->addWidget(m_streamButton, 0, 0);
  
  m_Vquality = new QLineEdit;
  m_Vquality->setValidator(new QIntValidator(1, 100, m_Vquality));
  m_Vquality->setText("15");		//default video quality
  
  QLabel *vQual = new QLabel("V quality");
  layoutG->addWidget(vQual, 1, 0);
  layoutG->addWidget(m_Vquality, 1, 1);
  
  m_Vbitrate = new QLineEdit;
  m_Vbitrate->setValidator(new QIntValidator(m_Vbitrate));
  m_Vbitrate->setText("100000");	//default video bitrate
  
  QLabel *vBps = new QLabel("V bitrate");
  layoutG->addWidget(vBps, 2, 0);
  layoutG->addWidget(m_Vbitrate, 2, 1);
  
  m_Aquality = new QLineEdit;
  m_Aquality->setValidator(new QIntValidator(1, 100, m_Aquality));
  m_Aquality->setText("15");		//default audio quality
  
  QLabel *aQual = new QLabel("A quality");
  layoutG->addWidget(aQual, 3, 0);
  layoutG->addWidget(m_Aquality, 3, 1);
  
  m_Abitrate = new QLineEdit;
  m_Abitrate->setValidator(new QIntValidator(9000, 320000, m_Abitrate));
  m_Abitrate->setText("96000");	//default audio bitrate
  
  QLabel *aBps = new QLabel("A bitrate");
  layoutG->addWidget(aBps, 4, 0);
  layoutG->addWidget(m_Abitrate, 4, 1);

  m_dumpButton = new QRadioButton("Dump", this);
  m_dumpButton->setAutoExclusive(false);
  layoutG->addWidget(m_dumpButton, 5, 0);

  m_FileName = new QLineEdit;
  m_FileName->setText("Video/dump.ogg");
  layoutG->addWidget(m_FileName, 5, 1);

  QLabel *host = new QLabel("Host");
  layoutG->addWidget(host, 6, 0);
  m_Host = new QLineEdit;
  m_Host->setText("localhost");
  layoutG->addWidget(m_Host, 6, 1);
  
  QLabel *port = new QLabel("Port");
  layoutG->addWidget(port, 7, 0);
  m_Port = new QLineEdit;
  m_Port->setValidator(new QIntValidator(1, 999999, m_Port));
  m_Port->setText("8000");
  layoutG->addWidget(m_Port, 7, 1);
  
  QLabel *shoutname = new QLabel("Name");
  layoutG->addWidget(shoutname, 8, 0);
  m_ShoutName = new QLineEdit;
  m_ShoutName->setText("qfreej streaming");
  layoutG->addWidget(m_ShoutName, 8, 1);
  
  QLabel *user = new QLabel("User");
  layoutG->addWidget(user, 9, 0);
  m_User = new QLineEdit;
  m_User->setText("source");
  layoutG->addWidget(m_User, 9, 1);
  
  QLabel *pass = new QLabel("Passwd");
  layoutG->addWidget(pass, 10, 0);
  m_Pass = new QLineEdit;
  m_Pass->setText("test!");
  layoutG->addWidget(m_Pass, 10, 1);

  QLabel *filename = new QLabel("File name");
  layoutG->addWidget(filename, 11, 0);
  m_ShoutFileName = new QLineEdit;
  m_ShoutFileName->setText("freejcpp.ogv");
  layoutG->addWidget(m_ShoutFileName, 11, 1);
  
  m_IceButton = new QRadioButton("IceCast", this);
  m_IceButton->setAutoExclusive(false);
  layoutG->addWidget(m_IceButton, 11, 2);
  m_IceButton->setChecked(true);

  setLayout(layoutG);
  setWindowTitle("Encoder");
}

void QEncoder::stream()
{
  if (!m_streaming)
  {
    m_streaming = true;
    m_enc=new OggTheoraEncoder();
    m_enc->video_quality = m_Vquality->text().toInt();
    m_enc->video_bitrate = m_Vbitrate->text().toInt();
    if (m_Qfreej->IsAudioOn())
    {
      m_Qjack = m_Qfreej->getQjack();
      if (m_enc->audio = m_Qjack->getAudio())
      {
	m_enc->use_audio = true;
	(m_Qjack->getJack())->isEncoded(false);
	m_enc->audio_quality = m_Aquality->text().toInt();
	m_enc->audio_bitrate = m_Abitrate->text().toInt();
      }
    }
    else
      m_enc->audio = NULL;

    if (m_dumpButton->isChecked())
    {
      m_enc->set_filedump(m_FileName->text().toAscii());
    }
    if (m_IceButton->isChecked())
    {
      if(shout_set_host(m_enc->ice, m_Host->text().toAscii()))
	qDebug() << "shout_set_host: " << shout_get_error(m_enc->ice);

      if(shout_set_port(m_enc->ice, m_Port->text().toInt()))
	qDebug() << "shout_set_port: " << shout_get_error(m_enc->ice);

      if(shout_set_name(m_enc->ice, m_ShoutName->text().toAscii()))
	qDebug() << "shout_set_title: " << shout_get_error(m_enc->ice);

      if(shout_set_user(m_enc->ice, m_User->text().toAscii()))
	qDebug() << "shout_set_user: " << shout_get_error(m_enc->ice);

      if(shout_set_password(m_enc->ice, m_Pass->text().toAscii()))
	qDebug() << "shout_set_pass: " << shout_get_error(m_enc->ice);

      if(shout_set_mount(m_enc->ice, m_ShoutFileName->text().toAscii()))
	qDebug() << "shout_set_mount: " << shout_get_error(m_enc->ice);
      if(shout_open(m_enc->ice) == SHOUTERR_SUCCESS) {
	qDebug() << "streaming on url: http://" << shout_get_host(m_enc->ice) << ":" \
	    << shout_get_port(m_enc->ice) <<  shout_get_mount(m_enc->ice);
	m_enc->write_to_stream = true;
      } else {
	qDebug() << "error connecting to server " << shout_get_host(m_enc->ice) << ":" \
	    << shout_get_error(m_enc->ice);
	m_enc->write_to_stream = false;
      }
    }
    m_freej->add_encoder(m_enc);	//also calls the OggTheoraEncoder::init method
    if (m_enc->audio)
      m_enc->audio->Jack->isEncoded(true);	//starts to fill the audio encoder ring buffer
    m_enc->active = true;
    m_streamButton->setText("Close to STOP");
  }
  else
  {
    m_streaming = false;
    m_enc->stop();
    m_enc->active = false;
    deleteLater();
  }
}

OggTheoraEncoder *QEncoder::getEnc ()
{
  return m_enc;
}

void QEncoder::closeEvent (QCloseEvent *ev)
{
  ev->accept();
}
