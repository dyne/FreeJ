#ifndef OPACITY_H
#define OPACITY_H

#include "qfreej.h"

class Qfreej;

class QOpacity : public QWidget
{
    Q_OBJECT
public:
    QOpacity(Qfreej*);
    ~QOpacity();

public slots:
  void changeOpacity(int);

private:
  double m_opacity;
  Qfreej *m_QfreejPtr;
};

#endif // OPACITY_H
