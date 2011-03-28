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
  m_Freej = qfreej->getFreej();
  m_Qfreej = qfreej;
  setAttribute(Qt::WA_QuitOnClose, false);
  setAttribute(Qt::WA_DeleteOnClose);
  if (init())
  {
    m_JackIsOn = true;
    m_Qfreej->setsAudioVar(true);
    this->show();
  }
  else
  {
    QMessageBox::information(this, "Jack Status","Daemon jackd doesn't seems to be started");
  }
}

QJackClient::~QJackClient()
{
  if (m_JackIsOn && m_Jack) m_Jack->Detach();
  m_Jack = NULL;
  m_Qfreej->resetQJack();
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

  m_audio = new AudioCollector(1024, 48192, m_Jack);		//48240 seems to be a good value in my config
//   m_audio = new AudioCollector(1024, 48000, m_Jack);
  m_Jack->isEncoded(false);
  m_Enc = m_Qfreej->getEnc();

  QLabel *jackStatusText = new QLabel("Jack's On, 48000:1024");
  layoutG->addWidget(jackStatusText, 0, 0);
  
  QPushButton *addButton = new QPushButton("Add Input", this);
  connect (addButton, SIGNAL(clicked()), this, SLOT(addInput()));
  layoutG->addWidget(addButton, 1, 0);
  addButton->setToolTip("Only one input available");
  m_InputNumbers = new QLCDNumber(this);
  layoutG->addWidget(m_InputNumbers, 1, 1);
  
  m_Coef = new QSlider(this);
  connect(m_Coef, SIGNAL(sliderMoved(int)), this, SLOT(changeMixCoef(int)));
  m_Coef->setOrientation(Qt::Horizontal);
  m_Coef->setRange(0,300);
  m_Coef->setValue(100);
  m_Coef->setEnabled(false);
  layoutG->addWidget(m_Coef, 2, 0, 1, 2);
  m_Coef->setToolTip("sets the Mix coefficient between Jack Input and Video Layer output");

  //not used
  m_SampleRate = new QLineEdit;
//   connect (m_SampleRate, SIGNAL(returnPressed()), this, SLOT(chgSampleRate()));
  m_SampleRate->setValidator(new QIntValidator(m_SampleRate));
  m_SampleRate->setText("48000");	//default Jackd sample rate
  m_SampleRate->setEnabled(false);
//   m_Jack->m_SampleRate = 48000;	//not necessary as it is donne in AudioCollector constructor
  //
  QLabel *vSampleRate = new QLabel("J SampleRate :");
  layoutG->addWidget(vSampleRate, 3, 0);
  layoutG->addWidget(m_SampleRate, 3, 1);
  
  m_Samples = new QLineEdit;
  connect (m_Samples, SIGNAL(returnPressed()), this, SLOT(chgSamples()));
  m_Samples->setValidator(new QIntValidator(m_Samples));
  m_Samples->setText("1024");		//default value : 1024 samples
  m_Samples->setEnabled(false);
  m_Jack->m_BufferSize = 1024;

  QLabel *vSample = new QLabel("J Samples :");
  layoutG->addWidget(vSample, 4, 0);
  layoutG->addWidget(m_Samples, 4, 1);
  
  QPushButton *outputButton = new QPushButton("Add Output(s)", this);
  connect (outputButton, SIGNAL(clicked()), this, SLOT(addOutput()));
  layoutG->addWidget(outputButton, 5, 0);

  m_nbrChan = new QSpinBox(0);
  layoutG->addWidget(m_nbrChan, 5, 1);

  this->setLayout(layoutG);
  this->setWindowTitle("Jack bridge");
  return (true);
}

void QJackClient::changeMixCoef(int val)
{
  if ((m_Enc = m_Qfreej->getEnc()))
  {
    float value = (float)val / 100.0;
    m_Enc->setMixCoef(value);
  }
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
    if ((m_Enc = m_Qfreej->getEnc()))
    {
      m_Enc->audio = m_audio;
      m_Enc->use_audio = true;
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
    m_InputNumbers->display(m_Inputs);
    m_Coef->setEnabled(true);
    if ((m_Enc = m_Qfreej->getEnc()))
    {
      m_Enc->audio = m_audio;
      m_Enc->use_audio = true;
    }
  }
}

// void QJackClient::chgSampleRate()
// {
//   m_Jack->m_SampleRate = m_SampleRate->text().toInt();
// }

int QJackClient::getSampleRate()
{
  return (m_SampleRate->text().toInt());
}

void QJackClient::chgSamples()
{
  m_Jack->m_BufferSize = m_Samples->text().toInt();
}
