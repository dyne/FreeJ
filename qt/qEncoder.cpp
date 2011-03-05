#include <qEncoder.h>
#include <QDebug>

QEncoder::QEncoder(Qfreej *qfreej) : QWidget()
{
  setAttribute(Qt::WA_DeleteOnClose);
  m_enc=new OggTheoraEncoder();
  m_enc->audio = NULL;
  m_Qjack = NULL;
  m_streaming = false;
  m_Qfreej = qfreej;
  m_freej = qfreej->getFreej();
  init();
  if (qfreej->IsAudioOn())
  {
    m_Qjack = qfreej->getQjack();
    m_enc->audio = m_Qjack->getAudio();
    m_enc->use_audio = true;
    (m_Qjack->getJack())->isEncoded(false);
  }
  m_enc->active = false;
  m_freej->add_encoder(m_enc);
  m_enc->stop();
  show();
}

QEncoder::~QEncoder()
{
  if (m_Qjack)
    (m_Qjack->getJack())->isEncoded(false);
  delete m_enc;
  m_Qfreej->resetEnc();
}

void QEncoder::init()
{
  QGridLayout *layoutG = new QGridLayout;
  
  QPushButton *chgButton = new QPushButton("Change Prm", this);
  connect (chgButton, SIGNAL(clicked()), this, SLOT(chgParam()));
  layoutG->addWidget(chgButton, 0, 0);

  m_streamButton = new QPushButton("Stream START", this);
  connect (m_streamButton, SIGNAL(clicked()), this, SLOT(stream()));
  layoutG->addWidget(m_streamButton, 0, 1);
  
  m_Vquality = new QLineEdit;
  connect (m_Vquality, SIGNAL(returnPressed()), this, SLOT(chgVquality()));
  m_Vquality->setValidator(new QIntValidator(1, 100, m_Vquality));
  m_Vquality->setText("15");		//default video quality
  
  QLabel *vQual = new QLabel("V quality :");
  layoutG->addWidget(vQual, 1, 0);
  layoutG->addWidget(m_Vquality, 1, 1);
  
  m_Vbitrate = new QLineEdit;
  connect (m_Vbitrate, SIGNAL(returnPressed()), this, SLOT(chgVbitrate()));
  m_Vbitrate->setValidator(new QIntValidator(m_Vbitrate));
  m_Vbitrate->setText("200000");	//default video bitrate
  
  QLabel *vBps = new QLabel("V bitrate :");
  layoutG->addWidget(vBps, 2, 0);
  layoutG->addWidget(m_Vbitrate, 2, 1);
  
  m_enc->video_quality = 15;
  m_enc->video_bitrate = 200000;
  
  m_dumpButton = new QRadioButton("Dump", this);
  layoutG->addWidget(m_dumpButton, 3, 0);

  m_FileName = new QLineEdit;
  m_FileName->setText("Video/dump.ogg");	//default video bitrate
  layoutG->addWidget(m_FileName, 3, 1);

  //needs to implement audio_quality and bitrate

  setLayout(layoutG);
  setWindowTitle("Encoder");
}

void QEncoder::chgVquality()
{
  m_enc->video_quality = m_Vquality->text().toInt();
}

void QEncoder::chgVbitrate()
{
  m_enc->video_bitrate = m_Vbitrate->text().toInt();
}

void QEncoder::chgParam()
{
}

void QEncoder::stream()
{
  if (!m_streaming)
  {
    m_streaming = true;
    if (m_dumpButton->isChecked())
    {
      QString txt = m_FileName->text();
      m_enc->set_filedump(txt.toStdString().c_str());
    }
    if (m_enc->audio)
      m_enc->audio->Jack->isEncoded(true);
    m_enc->start();
    m_enc->active = true;
    m_streamButton->setText("Stream STOP");
  }
  else
  {
    m_streaming = false;
    m_enc->stop();
    if (m_enc->audio)
      m_enc->audio->Jack->isEncoded(false);
    m_enc->active = false;
    m_streamButton->setText("Stream START");
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