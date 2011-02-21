#include "qOpacity.h"

QOpacity::QOpacity(QWidget *parent) : QWidget(parent)
{
  m_opacity = 1;
  QVBoxLayout *layoutV = new QVBoxLayout;
  QHBoxLayout* layoutH = new QHBoxLayout;
  QSlider *slider = new QSlider(this);
  slider->setSliderDown(true);
  slider->setRange(0,100);
  layoutH->addWidget(slider);
  layoutV->addLayout(layoutH);
  setLayout(layoutV);
  
  m_QfreejPtr = (Qfreej *)parent;
  connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(changeOpacity(int)));
}

QOpacity::~QOpacity()
{
}

void QOpacity::changeOpacity(int val)
{
  m_opacity = (double)val / 100.0;
  m_QfreejPtr->setWindowOpacity(m_opacity);
}