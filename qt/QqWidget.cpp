#include <QqWidget.h>
#include <iostream>
#include <QTabWidget>
#include <QMessageBox>
#include "qfreej.h"
#include <QqComboBlit.h>
#include <QqComboFilter.h>
#include <QDebug>
#include <QGridLayout>
#include <specialeventget.h>
#include <QFile>
#include <QDial>


extern QSize viewSize;

using namespace std;

QqTabWidget::QqTabWidget(QWidget* parent) : QTabWidget(parent)
{
    setToolTip("you can change Layers order by sliding tabs");
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(setSize(int)));
}

QqTabWidget::~QqTabWidget()
{
}

QTabBar *QqTabWidget::getTabBar()
{
    return (tabBar());
}

void QqTabWidget::moveLayer(int from, int to)
{
    if (QqWidget *widg = qobject_cast<QqWidget *>(this->widget(from)))
    {
        Context* ctx = widg->getContext();
        int newFrom = from;
        newFrom++;

        if (Layer* lay = widg->getLayer())
            lay->move(newFrom);
        else if (TextLayer* textlay = widg->getTextLayer())
            textlay->move(newFrom);
        ctx->screen->clear();
    }
}

void QqTabWidget::closeTab(int idx)
{
    if (QqWidget *widg = qobject_cast<QqWidget *>(this->widget(idx)))
    {
        widg->close();
    }
    removeTab(idx);
  }

//keeps all fake viewport the same size.
void QqTabWidget::setSize(int idx)
{
    if (idx > 0)
    {
        QqWidget* widg = (QqWidget *)widget(idx);
        if (widg)
        {
            FakeWindow* fake = widg->getFake();
            fake->resize(viewSize);
        }
    }
}

FakeWindow::FakeWindow(Context *context, Layer *layer, Geometry *geo, QWidget* parent) : QWidget(parent)
{
    qContext = context;
    qLayer = layer;
    qTextLayer = NULL;
    winGeo = new QRect(geo->x, geo->y, (int)geo->w, (int)geo->h);
    setGeometry(*winGeo);
}

FakeWindow::FakeWindow(Context *context, TextLayer *textlayer, Geometry *geo, QWidget *parent) : QWidget(parent)
{
    qContext = context;
    qTextLayer = textlayer;
    qLayer = NULL;
    winGeo = new QRect(geo->x, geo->y, (int)geo->w, (int)geo->h);
    setGeometry(*winGeo);
}

FakeWindow::~FakeWindow()
{
    delete winGeo;
}

Context* FakeWindow::getContext()
{
    return(qContext);
}

Layer* FakeWindow::getLayer()
{
    return(qLayer);
}

TextLayer* FakeWindow::getTextLayer()
{
    return(qTextLayer);
}

QRect* FakeWindow::getWinGeo()
{
    return(winGeo);
}

//Layer
QqWidget::QqWidget(Context *freej, QqTabWidget* tabWidget, Qfreej* qfreej, QString fichier) : QWidget(qfreej)
{
    qTextLayer = NULL;
    qLayer = freej->open((char*)fichier.toStdString().c_str(), 0, 0); // hey, this already init and open the layer !!

    if(qLayer)
    {
        if( freej->screen->add_layer(qLayer) )
        {
            qLayer->start();

            if (qLayer->frame_rate > 50)   //pb de determination de FPS
            {
                qLayer->fps.set(qLayer->frame_rate / 10);
                normalFps = qLayer->frame_rate / 10;
                actualFps = qLayer->frame_rate / 10;
            }
            else
            {
                qLayer->fps.set(qLayer->frame_rate);
                normalFps = qLayer->frame_rate;
                actualFps = qLayer->frame_rate;
            }
            slowFps = normalFps / 2;
            tabWidget->addTab(this, qLayer->get_filename());
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
    newIdx = 0;

    layoutV = new QVBoxLayout;
    layoutH = new QHBoxLayout;

    QqComboBlit *blt = new QqComboBlit(this);
    blt->addLayer(qLayer);

    slowButton = new QPushButton("Slow",this);
    connect (slowButton, SIGNAL(clicked()), this, SLOT(slowDown()));

    layoutH->addWidget(blt);
    QqComboFilter *filter = new QqComboFilter(freej, qLayer, this);

    filter->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);  // on the highter widget to fix the maximum hight
    layoutH->addWidget(filter);                                         // off the layer

    layoutH->addWidget(slowButton);
    QPushButton *cleanButton = new QPushButton("Clean", this);
    connect (cleanButton, SIGNAL(clicked()), this, SLOT(clean()));
    layoutH->addWidget(cleanButton);

    QDial* dial = new QDial(this);
    layoutH->addWidget(dial);

    layoutV->addLayout(layoutH);


    QWidget *bg = new QWidget(this);
    bg->setMinimumWidth(640);
    bg->setMinimumHeight(480);
    bg->setStyleSheet("QWidget { background-color: black; }");
    layoutV->addWidget(bg);


    fakeView = new FakeWindow(freej, (Layer *)NULL, &freej->screen->geo, bg);
    fakeView->setStyleSheet("QWidget { background-color: blue; }");
    SpecialEventGet* eventGet = new SpecialEventGet(this);
    fakeView->installEventFilter(eventGet);
    fakeView->setToolTip("Drag Left button to move Viewport, Drag center to resize");


    FakeWindow *fakeLay = new FakeWindow(freej, qLayer, &qLayer->geo, fakeView);
    fakeLay->setStyleSheet("QWidget { background-color: red; }");
    fakeLay->installEventFilter(eventGet);
    fakeLay->setToolTip("Drag Left button to move Layer, Drag center to resize");

    setLayout(layoutV);
}

// TextLayer
QqWidget::QqWidget(Context *freej, QqTabWidget *tabWidget, Qfreej* qfreej) : QWidget(qfreej)
{
    qLayer = NULL;
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
    connect(textButton, SIGNAL(clicked()), this, SLOT(modTextLayer()));
    layoutH->addWidget(textButton);

    QComboBox *fontSizeBox = new QComboBox(this);
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
    layoutH->addWidget(filter);

    layoutV->addLayout(layoutH);
    QTextEdit *texto = new QTextEdit(this);
    text = texto;
    texto->setMaximumHeight(40);
    layoutV->addWidget(texto);

    QWidget *bg = new QWidget(this);
    bg->setMinimumWidth(640);
    bg->setMinimumHeight(480);
    bg->setStyleSheet("QWidget { background-color: black; }");
    layoutV->addWidget(bg);


    fakeView = new FakeWindow(freej, (Layer *)NULL, &freej->screen->geo, bg);
    fakeView->setStyleSheet("QWidget { background-color: blue; }");
    SpecialEventGet* eventGet = new SpecialEventGet(this);
    fakeView->installEventFilter(eventGet);
    fakeView->setToolTip("Drag Left button to move Viewport, Drag center to resize");

    FakeWindow *fakeLay = new FakeWindow(freej, qTextLayer, &qTextLayer->geo, fakeView);
    fakeLay->setStyleSheet("QWidget { background-color: red; }");
    fakeLay->installEventFilter(eventGet);
    fakeLay->setToolTip("Drag Left button to move Layer, Drag center to resize");

    setLayout(layoutV);
}

QqWidget::~QqWidget()
{
    delete layoutH;
    delete layoutV;
    ctx->screen->clear();
//    Entry *le, *fe;

//    bool res = false;

/*
    if(ctx->screens.selected()->layers.len() > 0) { // there are layers

        res = true;

        // get the one selected
        le = ctx->screens.selected()->layers.selected();
        if(!le) {
            ctx->screens.selected()->layers.begin();
            le->sel(true);
        }

        ctx->rem_layer( (Layer*)le );
    }
*/
    if (qTextLayer)
    {
        if (qTextLayer->active)
            ctx->rem_layer(qTextLayer);
    }
    else if (qLayer)
    {
        if (qLayer->active)
            ctx->rem_layer(qLayer);
    }
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

FakeWindow* QqWidget::getFake()
{
    return(fakeView);
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

void QqWidget::slowDown()
{
    //normal speed devided by two
    if (qLayer)
    {
        if (actualFps != slowFps)
        {
            qLayer->fps.set(slowFps);
            cout << "slow down to : " << slowFps << " fps" << "actual fps : " << actualFps << endl;
            actualFps = slowFps;
            slowButton->setText("Normal");
        }
        else
        {
            qLayer->fps.set(normalFps);
            cout << "speed up to : " << normalFps << " fps" << endl;
            actualFps = normalFps;
            slowButton->setText("Slow");
        }
    }
}

Context* QqWidget::getContext()
{
    return(ctx);
}
