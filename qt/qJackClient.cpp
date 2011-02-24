#include <qJackClient.h>
#include <QDebug>

QJackClient::QJackClient(Context *freej) : QWidget()
{
  m_Jack = NULL;
  m_SampleRate = NULL;
  m_Samples = NULL;
  m_JackIsOn = false;
  m_Inputs = 0;
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
  if (m_JackIsOn && m_Jack) m_Jack->Detach();
}

bool QJackClient::init()
{
  QGridLayout *layoutG = new QGridLayout;
  
  m_Jack = JackClient::Get();
  if (!m_Jack->Attach("qfreej"))
  {
    return (false);
  }
  
  QLabel jackStatusText("Jack's On");
  layoutG->addWidget(&jackStatusText, 0, 0);
  
  QPushButton *addButton = new QPushButton("Add Input", this);
  connect (addButton, SIGNAL(clicked()), this, SLOT(addInput()));
  layoutG->addWidget(addButton, 1, 0);
  
  m_SampleRate = new QLineEdit;
  connect (m_SampleRate, SIGNAL(returnPressed()), this, SLOT(chgSampleRate()));
  m_SampleRate->setValidator(new QIntValidator(m_SampleRate));
  m_SampleRate->setText("48000");	//default Jackd sample rate
  layoutG->addWidget(m_SampleRate, 2, 0);
  
  m_Samples = new QLineEdit;
  connect (m_Samples, SIGNAL(returnPressed()), this, SLOT(chgSamples()));
  m_Samples->setValidator(new QIntValidator(m_Samples));
  m_Samples->setText("1024");		//default value : 1024 samples
  layoutG->addWidget(m_Samples, 3, 0);
  
  m_Jack->m_SampleRate = 48000;
  m_Jack->m_BufferSize = 1024;
  this->setLayout(layoutG);
  this->setWindowTitle("Jack bridge");
  return (true);
}

bool QJackClient::isOn()
{
  return (m_JackIsOn);
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

void QJackClient::chgSamples()
{
  m_Jack->m_BufferSize = m_Samples->text().toInt();
}
