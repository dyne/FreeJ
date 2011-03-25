#ifndef QENC_H
#define QENC_H
#include <oggtheora_encoder.h>
#include "qfreej.h"
#include <QtGui>
#include <qJackClient.h>

class QJackClient;

class QEncoder : public QWidget
{
    Q_OBJECT
public:
    QEncoder(Qfreej *);
    ~QEncoder();
    OggTheoraEncoder *getEnc ();


public slots:
  void stream();
  void updateStreamRate();
  
private:
  void init();
  QLineEdit *m_Vquality;
  QLineEdit *m_Vbitrate;
  QLineEdit *m_Aquality;
  QLineEdit *m_Abitrate;
  QLineEdit *m_Bitrate;
  QLineEdit *m_FileName;
  Qfreej *m_Qfreej;
  Context *m_freej;
  bool m_streaming;
  QRadioButton *m_dumpButton;
  QPushButton *m_streamButton;
  QJackClient *m_Qjack;
  OggTheoraEncoder *m_enc;
  QLineEdit *m_Host;
  QLineEdit *m_Port;
  QLineEdit *m_ShoutName;
  QLineEdit *m_User;
  QLineEdit *m_Pass;
  QLineEdit *m_ShoutFileName;
  QRadioButton *m_IceButton;
  QTimer *pollerRate;


protected:
  virtual void closeEvent (QCloseEvent *ev);
};

#endif // QENC_H
