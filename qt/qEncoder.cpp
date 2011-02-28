#include <qEncoder.h>
#include <QDebug>

QEncoder::QEncoder(Qfreej *qfreej) : QWidget(), OggTheoraEncoder()
{
  audio = NULL;
  init();
  if (qfreej->IsAudioOn())
  {
    audio = new AudioCollector(1024, 48000);
    use_audio = true;
    QJackClient *Qjack = qfreej->getQjack();
    Qjack->getJack()->isEncoded(true);
    qDebug() << "--- use_audio true, et audio";
  }
  this->show();
}

QEncoder::~QEncoder()
{
  if (audio) delete (audio);
}

AudioCollector *QEncoder::getAudio()
{
  if (audio) return (audio);
  else return (NULL);
}

bool QEncoder::init()
{
  QGridLayout *layoutG = new QGridLayout;
  
  QPushButton *chgButton = new QPushButton("Change", this);
  connect (chgButton, SIGNAL(clicked()), this, SLOT(chgParam()));
  layoutG->addWidget(chgButton, 0, 0);
  
  m_Vquality = new QLineEdit;
  connect (m_Vquality, SIGNAL(returnPressed()), this, SLOT(chgVquality()));
  m_Vquality->setValidator(new QIntValidator(1, 100, m_Vquality));
  m_Vquality->setText("15");		//default video quality
  layoutG->addWidget(m_Vquality, 1, 0);
  
  m_Vbitrate = new QLineEdit;
  connect (m_Vbitrate, SIGNAL(returnPressed()), this, SLOT(chgVbitrate()));
  m_Vbitrate->setValidator(new QIntValidator(m_Vbitrate));
  m_Vbitrate->setText("200000");	//default video bitrate
  layoutG->addWidget(m_Vbitrate, 2, 0);
  
  video_quality = 15;
  video_bitrate = 200000;
  
  QRadioButton *dumpButton = new QRadioButton("Dump", this);
  connect (dumpButton, SIGNAL(clicked()), this, SLOT(dumpSlot()));
  layoutG->addWidget(dumpButton, 3, 0);

  m_FileName = new QLineEdit;
  m_FileName->setText("Video/dump.ogg");	//default video bitrate
  layoutG->addWidget(m_FileName, 3, 1);

  //needs to implement audio_quality and bitrate

  this->setLayout(layoutG);
  this->setWindowTitle("Encoder");
  return (true);
}

void QEncoder::chgVquality()
{
  video_quality = m_Vquality->text().toInt();
}

void QEncoder::chgVbitrate()
{
  video_bitrate = m_Vbitrate->text().toInt();
}

void QEncoder::chgParam()
{
}

void QEncoder::dumpSlot()
{
  if (!write_to_disk)
  {
    QString txt = m_FileName->text();
    set_filedump(txt.toStdString().c_str());
  }
  else
  {
    filedump_close();
  }
}