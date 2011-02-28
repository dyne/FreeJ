#ifndef QENC_H
#define QENC_H
#include <oggtheora_encoder.h>
#include "qfreej.h"
#include <QtGui>

class QEncoder : public QWidget, public OggTheoraEncoder
{
    Q_OBJECT
public:
    QEncoder(Qfreej *);
    ~QEncoder();
    AudioCollector *getAudio();

public slots:
  void chgParam();
  void chgVquality();
  void chgVbitrate();
  void dumpSlot();
  
private:
  bool init();
  QLineEdit *m_Vquality;
  QLineEdit *m_Vbitrate;
  QLineEdit *m_FileName;
};

#endif // QENC_H
