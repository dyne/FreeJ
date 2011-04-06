/*  Qfreej
 *  (c) Copyright 2010 fred_99
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */



#include <QqWidget.h>
#include <iostream>
#include <QTabWidget>
#include <QMessageBox>
#include <qfreej.h>
#include <QqComboBlit.h>
#include <QqComboFilter.h>
#include <QDebug>
#include <QGridLayout>
#include <QFile>
#include <QDial>
#include <specialeventget.h>
#include <QAction>

using namespace std;

//Layer
QqWidget::QqWidget(Context *freej, QqTabWidget* tabWidget, Qfreej* qfreej, QString fichier) : QWidget(qfreej)
{
    m_tabWidg = NULL;
    qTextLayer = NULL;
    m_qGeneLayer = NULL;
    fakeView = NULL;
    fakeLay = NULL;
    ctx = NULL;
    actualFps = normalFps = fpsP = 0;
    qLayer = freej->open((char*)fichier.toStdString().c_str(), 0, 0); // hey, this already init and open the layer !!
    if(qLayer)
    {
        if( freej->screen->add_layer(qLayer) )
        {
            qLayer->start();	//launches JSyncThread::start()

/*            if (qLayer->frame_rate > 50)   //pb de determination de FPS
            {
				std::cout << "--------- qfreej : frame_rate problem : " << qLayer->frame_rate << " FPS" << std::endl;
                qLayer->fps.set(qLayer->frame_rate / 10); 
                normalFps = qLayer->frame_rate / 10;
                actualFps = qLayer->frame_rate / 10;
            }
            else
            {*/
            qLayer->fps.set(qLayer->frame_rate);
            normalFps = qLayer->frame_rate;
            actualFps = qLayer->frame_rate;
	    fpsP = 100;
	    qDebug() << "--- actualFps :" << actualFps;
//             }
            tabWidget->addTab(this, qLayer->get_filename());
            qLayer->move(freej->screen->layers.len());      //put the layer at the end of the list
            m_tabWidg = tabWidget;
	    if (QJackClient *qjack = qfreej->getQjack())
	    {
	      if (!qjack->howManyOutputs())
	      {
		(qjack->getJack())->SetRingbufferPtr(freej->screen->audio, qjack->getSampleRate()
		    , ((VideoLayer *)qLayer)->audio_channels);
		qjack->setNoutputs(((VideoLayer *)qLayer)->audio_channels);
	      }
	    }
        }
        else
        {
            if (!qfreej->getStartState())
                qLayer->active = false;
            QMessageBox::information(this, "Layers", "Can't create Layer :" + fichier);
            return;
        }
    }
    else
    {
        QMessageBox::information(this, "Layers", "Impossible to create TextLayer :" + fichier);
        return;
    }



    ctx = freej;
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_QuitOnClose, false);
    newIdx = 0;

    layoutV = new QVBoxLayout;
    layoutH = new QHBoxLayout;

    QqComboBlit *blt = new QqComboBlit(this);
    blt->addLayer(qLayer);



    layoutH->addWidget(blt);
    QqComboFilter *filter = new QqComboFilter(freej, qLayer, this);
    filter->setToolTip("filters to be applied");

    filter->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);  // on the highter widget to fix the maximum hight
    layoutH->addWidget(filter);                                         // off the layer

    if (actualFps)
    {
      slowFps = new QSlider(this);
      slowFps->setOrientation(Qt::Vertical);
      connect(slowFps, SIGNAL(sliderMoved(int)), this, SLOT(changeFps(int)));
      slowFps->setRange(2, 100);
      slowFps->setToolTip("speed control (grey: normal speed, red: higher or lower)");
      slowFps->setValue(50);
      slowFps->setSliderDown(true);
      slowFps->setStyleSheet("background: grey");
      layoutH->addWidget(slowFps);
    }

    QPushButton *cleanButton = new QPushButton("Clean", this);
    cleanButton->setToolTip("cleans the screen");
    connect (cleanButton, SIGNAL(clicked()), this, SLOT(clean()));
    layoutH->addWidget(cleanButton);


    m_angleBox = new QDoubleSpinBox(this);
    connect(m_angleBox, SIGNAL(valueChanged(double)), this, SLOT(changeAngle(double)));
    connect(m_angleBox, SIGNAL(editingFinished()), this, SLOT(redrawFake()));
    m_angleBox->setMinimum(-360.0);
    m_angleBox->setMaximum(360.0);
    m_angleBox->setDecimals(0);
    m_angleBox->setSingleStep(1.0);
    m_angleBox->setToolTip("rotate the layer\npress ENTER to update fake");
    layoutH->addWidget(m_angleBox);

    playButton = new QPushButton("PAUSE", this);
    connect (playButton, SIGNAL(clicked()), this, SLOT(playPause()));
    layoutH->addWidget(playButton);
    isPlaying = true;

    QPushButton* zoom = new QPushButton("Reset Zoom", this);
    connect (zoom, SIGNAL(clicked()), this, SLOT(resetZoom()));
    layoutH->addWidget(zoom);

    layoutV->addLayout(layoutH);

    QWidget *bg = new QWidget(this);
    bg->setMinimumWidth(400);
    bg->setMinimumHeight(300);
    bg->setStyleSheet("QWidget { background-color: black; }");
    layoutV->addWidget(bg);


    fakeView = new FakeWindow(freej, (Layer *)NULL, &freej->screen->geo, bg);
    fakeView->setStyleSheet("QWidget { background-color: blue; }");
    SpecialEventGet* eventGet = new SpecialEventGet(this);
    fakeView->installEventFilter(eventGet);
    fakeView->setToolTip("Drag right button to resize screen");


    fakeLay = new FakeWindow(freej, qLayer, &qLayer->geo, fakeView);
    fakeLay->installEventFilter(eventGet);
    fakeLay->setEventGet(eventGet);
    fakeLay->setToolTip("Drag Left button to move Layer, Drag right to resize");

    setLayout(layoutV);
}

// TextLayer
QqWidget::QqWidget(Context *freej, QqTabWidget *tabWidget, Qfreej* qfreej) : QWidget(qfreej)
{
    qLayer = NULL;
    m_qGeneLayer = NULL;
    fakeView = NULL;
    fakeLay = NULL;
    ctx = NULL;
    QString filename = "textouille.txt";
    QFile file( filename );
    if (file.open(QIODevice::WriteOnly))
    {
        file.close();
    }
    else
    {
        qDebug() << "Not possible to open or create textouille.txt";
        return;
    }
    qTextLayer = (TextLayer *)freej->open((char *)"textouille.txt", 0, 0);
    if (qTextLayer)
    {
        qTextLayer->init(200, 50, 32);
        qTextLayer->set_position(50, 200);
        qTextLayer->set_font("sans");
        if( freej->screen->add_layer(qTextLayer) )  //essayer sans ça plus tard
        {
            qTextLayer->start();

            qTextLayer->fps.set(qTextLayer->frame_rate);

            tabWidget->addTab(this, "text zone");
            qTextLayer->move(freej->screen->layers.len());      //put the layer at the end of the list
            m_tabWidg = tabWidget;
        }
        else
        {
            QMessageBox::information(this, "Layers", "Can't create TextLayer\n");
            if (!qfreej->getStartState())
                qTextLayer->active = false;
            return;
        }
    }
    else
    {
        QMessageBox::information(this, "Layers", "Impossible de créer la TextLayer\n");
        return;
    }

    ctx = freej;
    setAttribute(Qt::WA_DeleteOnClose);
    newIdx = 0;

    layoutV = new QVBoxLayout;
    layoutH = new QHBoxLayout;

    textButton = new QPushButton ("Apply");
    textButton->setToolTip("apply text on the textlayer");
    connect(textButton, SIGNAL(clicked()), this, SLOT(modTextLayer()));
    layoutH->addWidget(textButton);

    QComboBox *fontSizeBox = new QComboBox(this);
    fontSizeBox->setToolTip("set Font size");
    for (int i=40;i>=5;i--)
    {
        fontSizeBox->insertItem(0, QString::number(i));
    }
    qTextLayer->set_fontsize(15);
    fontSizeBox->setCurrentIndex(10);
    connect(fontSizeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFontSize(int)));

    layoutH->addWidget(fontSizeBox);
    QqComboBlit *blt = new QqComboBlit(this);
    blt->addTextLayer(qTextLayer);

    layoutH->addWidget(blt);
    QqComboFilter *filter = new QqComboFilter(freej, qTextLayer, this);
    filter->setToolTip("filters to be applied");
    layoutH->addWidget(filter);

    m_angleBox = new QDoubleSpinBox(this);
    connect(m_angleBox, SIGNAL(valueChanged(double)), this, SLOT(changeAngle(double)));
    connect(m_angleBox, SIGNAL(editingFinished()), this, SLOT(redrawFake()));
    m_angleBox->setMinimum(-360.0);
    m_angleBox->setMaximum(360.0);
    m_angleBox->setDecimals(0);
    m_angleBox->setSingleStep(1.0);
    m_angleBox->setToolTip("rotate the layer\npress ENTER to update fake");
    layoutH->addWidget(m_angleBox);

    QPushButton* zoom = new QPushButton("Reset Zoom", this);
    connect (zoom, SIGNAL(clicked()), this, SLOT(resetZoom()));
    layoutH->addWidget(zoom);

    layoutV->addLayout(layoutH);
    QTextEdit *texto = new QTextEdit(this);
    texto->setToolTip("text to be applied on the textlayer");
    text = texto;
    texto->setMaximumHeight(40);
    layoutV->addWidget(texto);

    QWidget *bg = new QWidget(this);
    bg->setMinimumWidth(400);
    bg->setMinimumHeight(300);
    bg->setStyleSheet("QWidget { background-color: black; }");
    layoutV->addWidget(bg);


    fakeView = new FakeWindow(freej, (Layer *)NULL, &freej->screen->geo, bg);
    fakeView->setStyleSheet("QWidget { background-color: blue; }");
    SpecialEventGet* eventGet = new SpecialEventGet(this);
    fakeView->installEventFilter(eventGet);
    fakeView->setToolTip("Drag right button to resize screen");

    fakeLay = new FakeWindow(freej, qTextLayer, &qTextLayer->geo, fakeView);
    fakeLay->installEventFilter(eventGet);
    fakeLay->setEventGet(eventGet);
    fakeLay->setToolTip("Drag Left button to move TextLayer, Drag center to resize");

    setLayout(layoutV);
}

// GeneratorLayer
QqWidget::QqWidget(Context *freej, QqTabWidget *tabWidget, Qfreej* qfreej, QAction* action) : QWidget(qfreej)
{
    qTextLayer = NULL;
    qLayer = NULL;
    fakeView = NULL;
    fakeLay = NULL;
    ctx = NULL;

    m_qGeneLayer = new GeneratorLayer();
    if(!m_qGeneLayer)
    {
        return;
    }
    if(!m_qGeneLayer->init(freej->screens.selected()->geo.w,
                  freej->screens.selected()->geo.h,
                  freej->screens.selected()->geo.bpp)) {
      qDebug() << "can't initialize generator layer";
      delete m_qGeneLayer;
      return;
    }
    // this is something specific to the generator layer
    // it needs this from the environment..
    m_qGeneLayer->register_generators( &freej->generators );

    if(!m_qGeneLayer->open(action->text().toStdString().c_str())) {
      qDebug() << "generator" << action->text() << "is not found";
      delete m_qGeneLayer;
      return;
    }

    m_qGeneLayer->start();
    //  tmp->set_fps(env->fps_speed);

    if( freej->screen->add_layer(m_qGeneLayer) )  //essayer sans ça plus tard
    {
        m_qGeneLayer->active=true;

        tabWidget->addTab(this, action->text());
        m_qGeneLayer->move(freej->screen->layers.len());      //put the layer at the end of the list
        m_tabWidg = tabWidget;
    }
    else
    {
        freej->rem_layer(m_qGeneLayer);
        delete m_qGeneLayer;
        return;
    }

    ctx = freej;
    setAttribute(Qt::WA_DeleteOnClose);
    newIdx = 0;

    layoutV = new QVBoxLayout;
    layoutH = new QHBoxLayout;

    QqComboBlit *blt = new QqComboBlit(this);
    blt->addGeneLayer(m_qGeneLayer);

    layoutH->addWidget(blt);
    QqComboFilter *filter = new QqComboFilter(freej, m_qGeneLayer, this);
    filter->setToolTip("filters to be applied");
    layoutH->addWidget(filter);

    m_angleBox = new QDoubleSpinBox(this);
    connect(m_angleBox, SIGNAL(valueChanged(double)), this, SLOT(changeAngle(double)));
    connect(m_angleBox, SIGNAL(editingFinished()), this, SLOT(redrawFake()));
    m_angleBox->setMinimum(-360.0);
    m_angleBox->setMaximum(360.0);
    m_angleBox->setDecimals(0);
    m_angleBox->setSingleStep(1.0);
    m_angleBox->setToolTip("rotate the layer\npress ENTER to update fake");
    layoutH->addWidget(m_angleBox);

    QPushButton* zoom = new QPushButton("Reset Zoom", this);
    connect (zoom, SIGNAL(clicked()), this, SLOT(resetZoom()));
    layoutH->addWidget(zoom);

    layoutV->addLayout(layoutH);

    QWidget *bg = new QWidget(this);
    bg->setMinimumWidth(400);
    bg->setMinimumHeight(300);
    bg->setStyleSheet("QWidget { background-color: black; }");
    layoutV->addWidget(bg);


    fakeView = new FakeWindow(freej, (Layer *)NULL, &freej->screen->geo, bg);
    fakeView->setStyleSheet("QWidget { background-color: blue; }");
    SpecialEventGet* eventGet = new SpecialEventGet(this);
    fakeView->installEventFilter(eventGet);
    fakeView->setToolTip("Drag right button to resize screen");

    fakeLay = new FakeWindow(freej, m_qGeneLayer, &m_qGeneLayer->geo, fakeView);
    fakeLay->installEventFilter(eventGet);
    fakeLay->setEventGet(eventGet);
    fakeLay->setToolTip("Drag Left button to move GeneratorLayer, Drag center to resize");

    setLayout(layoutV);
}

QqWidget::~QqWidget()
{
  if (qTextLayer)
  {
    qTextLayer->stop();
    delete(qTextLayer);
  }
  else if (qLayer)
  {
    qLayer->stop();
    delete (qLayer);
  }
  else if (m_qGeneLayer)
  {
    m_qGeneLayer->stop();
    delete (m_qGeneLayer);
  }
}

void QqWidget::resetZoom()
{
    if (fakeLay)
    {
        fakeLay->resetZoom();
    }
}

void QqWidget::playPause()
{
    if (qLayer)
    {
        if (isPlaying)
        {
            qLayer->stop();
            isPlaying = false;
            playButton->setText("PLAY");
        }
        else
        {
            qLayer->start();
            isPlaying = true;
            playButton->setText("PAUSE");
        }
    }
}

//slot called when m_angleBox edit finished
void QqWidget::redrawFake()
{
    if (fakeLay)
    {
        double val = m_angleBox->value();
        fakeLay->setAngle((int)val);
        if (qLayer)
        {
            qLayer->set_rotate(-val);
        }
        else if (qTextLayer)
        {
            qTextLayer->set_rotate(-val);
        }
        fakeLay->repaint();
    }
}

//rotate Layer
void QqWidget::changeAngle(double val)
{
    if (qLayer)
        qLayer->set_rotate(-val);
    else if (qTextLayer)
        qTextLayer->set_rotate(-val);
    ctx->screen->clear();
    m_angle = val;
    if (fakeLay)
    {
        fakeLay->setAngle((int)val);
  //      fakeLay->repaint();
    }
}

void QqWidget::setAngle(double val)
{
    m_angle = val;
    m_angleBox->setValue(val);
}

double QqWidget::getAngle()
{
    return(m_angle);
}

//clean the view
void QqWidget::clean()
{
    ctx->screen->clear();
}

Layer* QqWidget::getLayer()
{
    return (qLayer);
}

TextLayer* QqWidget::getTextLayer()
{
    return (qTextLayer);
}

GeneratorLayer* QqWidget::getGeneLayer()
{
    return (m_qGeneLayer);
}

FakeWindow* QqWidget::getFake()
{
    if (fakeView)
        return(fakeView);
    else
        return (NULL);
}

void QqWidget::changeFontSize(int sizeIdx)
{
    int size = sizeIdx + 5;
    qTextLayer->set_fontsize(size);
}

void QqWidget::modTextLayer()
{
    QString txt = text->toPlainText();
    qTextLayer->write(txt.toStdString().c_str());
    ctx->screen->clear();
}

void QqWidget::changeFps(int val)
{
    if (qLayer)
    {
        if (val != 50)
        {
	  actualFps = ((float)val / 50.0) * normalFps;
	  qLayer->fps.set(actualFps);
	  slowFps->setStyleSheet("background: red");
        }
        else
	  slowFps->setStyleSheet("background: grey");
    }
}

Context* QqWidget::getContext()
{
    return(ctx);
}

QqTabWidget* QqWidget::getTabWidget()
{
    return (m_tabWidg);
}
