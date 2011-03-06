#ifndef QJACK_H
#define QJACK_H
#include <audio_jack.h>
#include <context.h>
#include <qEncoder.h>
#include <QtGui>

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
//     void sound();

private:
  bool init();
  JackClient *m_Jack;
  bool m_JackIsOn;
  QLineEdit *m_SampleRate;
  QLineEdit *m_Samples;
  int	m_Inputs;
  int	m_Outputs;
  Context *m_Freej;
  Qfreej *m_Qfreej;
  QSpinBox *m_nbrChan;
  AudioCollector *m_audio;
//   bool m_use_audio;
  OggTheoraEncoder *m_Enc;
  bool first;	//does QJackClient as been launched before QEncoder
};

#endif // QJACK_H
