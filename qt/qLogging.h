#ifndef QLOG_H
#define QLOG_H
#include <QtGui>
#include <logging.h>

class QLogger : public QWidget, public Logger
{
    Q_OBJECT
public:
    QLogger(QWidget *);
    ~QLogger();

public slots:
  
private:

};

#endif // QLOG_H
