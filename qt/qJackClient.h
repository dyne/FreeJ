#ifndef QJACK_H
#define QJACK_H
#include <audio_jack.h>
#include <context.h>
#include <qEncoder.h>
#include <QtGui>
#include <audio_collector.h>

class QEncoder;

class QJackClient : public QWidget
{
    Q_OBJECT
public:
    QJackClient(Qfreej *);
    ~QJackClient();
    
    bool isOn();
    JackClient *getJack();
    AudioCollector *getAudio();
    int howManyOutputs();
    int getSampleRate();
    void setNoutputs(int);

public slots:
  void addInput();
  void addOutput();
  void chgSampleRate();
  void chgSamples();

private:
  bool init();
  JackClient *m_Jack;
  bool m_JackIsOn;
  QLineEdit *m_SampleRate;
  QLineEdit *m_Samples;
  int	m_Inputs;
  QLCDNumber *m_InputNumbers;
  int	m_Outputs;
  Context *m_Freej;
  Qfreej *m_Qfreej;
  QSpinBox *m_nbrChan;
  AudioCollector *m_audio;
  OggTheoraEncoder *m_Enc;
};

#endif // QJACK_H
