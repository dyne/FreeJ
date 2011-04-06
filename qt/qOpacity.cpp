#include <qOpacity.h>

QOpacity::QOpacity(Qfreej *qfreej) : QWidget()
{
  m_opacity = 1;
  QVBoxLayout *layoutV = new QVBoxLayout;
  QHBoxLayout* layoutH = new QHBoxLayout;
  QSlider *slider = new QSlider(this);
  slider->setSliderDown(true);
  slider->setOrientation(Qt::Horizontal);
  slider->setRange(0,100);
  slider->setValue(100);
  layoutH->addWidget(slider);
  layoutV->addLayout(layoutH);
  setLayout(layoutV);
  
  m_QfreejPtr = qfreej;
  connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(changeOpacity(int)));
}

QOpacity::~QOpacity()
{
}

void QOpacity::changeOpacity(int val)
{
  m_opacity = (double)val / 100.0;
  if (m_QfreejPtr)
    m_QfreejPtr->setWindowOpacity(m_opacity);
}
