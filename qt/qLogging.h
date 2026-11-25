#ifndef QLOG_H
#define QLOG_H
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets>
#endif
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
