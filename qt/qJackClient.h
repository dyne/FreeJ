#ifndef QJACK_H
#define QJACK_H
#include <audio_layer.h>
#include <audio_jack.h>
#include <audio_collector.h>
#include <audio_input.h>
#include <oggtheora_encoder.h>
#include <QtGui>

class QJackClient : public QWidget
{
    Q_OBJECT
public:
    QJackClient(Context *);
    ~QJackClient();
    
    bool isOn();
    JackClient *getJack();

public slots:
  void addInput();
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
  Context *m_Freej;
};

#endif // QJACK_H
