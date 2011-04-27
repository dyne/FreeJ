#ifndef QQCOMBORES_H
#define QQCOMBORES_H

#include <QWidget>
#include <QComboBox>
#include <v4l2_layer.h>
#include <QqWidget.h>

class QqWidget;

class QqComboRes : public QWidget
{
    Q_OBJECT
public:
    explicit QqComboRes(V4L2CamLayer *, QqWidget *parent = 0);


public slots:
  void changeRes(int);

private:
    QComboBox *m_resBox;
    QqWidget *m_QqWidget;
    Res *m_res;
    V4L2CamLayer *m_lay;
    int m_nbRes;
};

#endif // QQCOMBORES_H
