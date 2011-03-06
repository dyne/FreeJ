#include <qJackClient.h>
#include <QDebug>

QJackClient::QJackClient(Qfreej *qfreej) : QWidget()
{
  m_Jack = NULL;
  m_SampleRate = NULL;
  m_Samples = NULL;
  m_audio = NULL;
  m_Enc = NULL;
  m_JackIsOn = false;
  m_Inputs = 0;
  m_Outputs = 0;
//   m_use_audio = false;
  m_Freej = qfreej->getFreej();
  m_Qfreej = qfreej;
  
  if (init())
  {
    m_JackIsOn = true;
    this->show();
  }
  else
  {
    QMessageBox::information(this, "Jack Status","Daemon jackd doesn't seems to be started");
  }
}

QJackClient::~QJackClient()
{
  if (m_audio)
  {
    m_Enc->audio = NULL;
    m_Enc->use_audio = false;
    delete (m_audio);
  }
  if (m_JackIsOn && m_Jack) m_Jack->Detach();
}

JackClient *QJackClient::getJack()
{
  return (m_Jack);
}

bool QJackClient::init()
{
  QGridLayout *layoutG = new QGridLayout;
  
  m_Jack = JackClient::Get();
  if (!m_Jack->Attach("qfreej"))
  {
    return (false);
  }

  m_audio = new AudioCollector(1024, 48000, m_Jack);
//   m_use_audio = true;
  m_Jack->isEncoded(false);
  if ((m_Enc = m_Qfreej->getEnc()))
  {
/*    m_Enc->audio = m_audio;
    m_Enc->use_audio = true;*/
    first = false;
  }
  else
    first = true;
//   if (!m_Freej->screen->add_audio(m_Jack))
//   {
//     QMessageBox::information(this, "Jack Output port","Can't create the Jack audio output ports");
//   }

  QLabel *jackStatusText = new QLabel("Jack's On");
  layoutG->addWidget(jackStatusText, 0, 0);
  
  QPushButton *addButton = new QPushButton("Add Input", this);
  connect (addButton, SIGNAL(clicked()), this, SLOT(addInput()));
  layoutG->addWidget(addButton, 1, 0);
  
  m_SampleRate = new QLineEdit;
  connect (m_SampleRate, SIGNAL(returnPressed()), this, SLOT(chgSampleRate()));
  m_SampleRate->setValidator(new QIntValidator(m_SampleRate));
  m_SampleRate->setText("48000");	//default Jackd sample rate

  QLabel *vSampleRate = new QLabel("J SampleRate :");
  layoutG->addWidget(vSampleRate, 2, 0);
  layoutG->addWidget(m_SampleRate, 2, 1);
  
  m_Samples = new QLineEdit;
  connect (m_Samples, SIGNAL(returnPressed()), this, SLOT(chgSamples()));
  m_Samples->setValidator(new QIntValidator(m_Samples));
  m_Samples->setText("1024");		//default value : 1024 samples

  QLabel *vSample = new QLabel("J Samples :");
  layoutG->addWidget(vSample, 3, 0);
  layoutG->addWidget(m_Samples, 3, 1);
  
  m_Jack->m_SampleRate = 48000;
  m_Jack->m_BufferSize = 1024;
  
  QPushButton *outputButton = new QPushButton("Add Output(s)", this);
  connect (outputButton, SIGNAL(clicked()), this, SLOT(addOutput()));
  layoutG->addWidget(outputButton, 4, 0);

  m_nbrChan = new QSpinBox(0);
  layoutG->addWidget(m_nbrChan, 4, 1);

  this->setLayout(layoutG);
  this->setWindowTitle("Jack bridge");
  return (true);
}

AudioCollector *QJackClient::getAudio()
{
  return (m_audio);
}

bool QJackClient::isOn()
{
  return (m_JackIsOn);
}

int QJackClient::howManyOutputs()
{
  return (m_Outputs);
}

void QJackClient::addOutput()
{
  if (!m_Outputs && m_audio)
  {
    m_Jack->SetRingbufferPtr(m_Freej->screen->audio, m_SampleRate->text().toInt(), m_nbrChan->value());
//     m_use_audio = true;
    if ((m_Enc = m_Qfreej->getEnc()) && first)
    {
      m_Enc->audio = m_audio;
      m_Enc->use_audio = true;
    }
    else if ((m_Enc = m_Qfreej->getEnc()))
    {
      QMessageBox::information(this, "Info", "If you want to stream the audio, you need to start \
	  \"Jack connext\" before \"Streaming\". Just quit Encoder and launch it again");
      m_Enc->audio = NULL;
      m_Enc->use_audio = false;
      qDebug() << "--- second !!!!!!";
    }
    m_Outputs += m_nbrChan->value();	//only one shot for the moment !!
  }
}

void QJackClient::setNoutputs(int n)
{
  m_Outputs = n;
  m_nbrChan->setValue(n);
}

void QJackClient::addInput()
{
  if (!m_Inputs)
  {
    m_Jack->AddInputPort();
    m_Inputs++;
  }
}

void QJackClient::chgSampleRate()
{
  m_Jack->m_SampleRate = m_SampleRate->text().toInt();
}

int QJackClient::getSampleRate()
{
  return (m_SampleRate->text().toInt());
}

void QJackClient::chgSamples()
{
  m_Jack->m_BufferSize = m_Samples->text().toInt();
}
