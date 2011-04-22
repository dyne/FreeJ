#include <qqcombores.h>
#include <sstream>
#include <string>
#include <QDebug>
#include <iostream>
#include <FakeWindow.h>

QqComboRes::QqComboRes(V4L2CamLayer *lay, QqWidget *parent) :
    QWidget(parent)
{
  m_QqWidget = parent;
  m_resBox = new QComboBox(this);
  m_resBox->setToolTip("V4L2 resolution");
  m_res = lay->getRes();
  m_lay = lay;
  QStringList res;
  
  QString qx, qy, qres;
  qDebug() << "++ nb:" << m_res->getNb();
  for (int i = 0; i < m_res->getNb(); i++){
    qx.setNum (m_res->getX(i));
    qy.setNum (m_res->getY(i));
    qDebug() << "++ x:" << m_res->getX(i) << " y:" << m_res->getY(i) << " i:" << i;
    qres.clear();
    qres = qx + "x" + qy;
    res << qres;
  }
  m_resBox->addItems(res);
  m_resBox->setCurrentIndex(m_res->getCurIdx());
  connect(m_resBox, SIGNAL(activated(int)), this, SLOT(changeRes(int)));
}

void QqComboRes::changeRes(int line) {
  m_lay->chgRes(line, m_res);
  m_QqWidget->resetZoom();
}