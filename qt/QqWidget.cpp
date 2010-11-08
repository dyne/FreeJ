#include <QqWidget.h>
#include <iostream>
#include <QTabWidget>
#include <QMessageBox>
#include <QAction>
#include "qfreej.h"
#include <QBoxLayout>
#include <QqComboBlit.h>
#include <QqComboFilter.h>
#include <QDebug>
#include <QGridLayout>
#include <specialeventget.h>


extern QSize viewSize;

using namespace std;

QqTabWidget::QqTabWidget() : QTabWidget()
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
        int newFrom = from;
        newFrom++;

        if (Layer* lay = widg->getLayer())
            lay->move(newFrom);
        else if (TextLayer* textlay = widg->getTextLayer())
            textlay->move(newFrom);
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
    if (idx >= 0)
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
QqWidget::QqWidget(Context *freej, Layer *lay) : QWidget()
{
    ctx = freej;
    setAttribute(Qt::WA_DeleteOnClose);
    isTextLayer = false;
    layerSet = false;
    newIdx = 0;

    QVBoxLayout *layoutV = new QVBoxLayout;
    QHBoxLayout *layoutH = new QHBoxLayout;

    addLayer (lay);

    QqComboBlit *blt = new QqComboBlit;
    blt->addLayer(lay);

    slowButton = new QPushButton("Slow");
    connect (slowButton, SIGNAL(clicked()), this, SLOT(slowDown()));

    layoutH->addWidget(blt);
    QqComboFilter *filter = new QqComboFilter(freej, lay);

    filter->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);  // on the highter widget to fix the maximum hight
    layoutH->addWidget(filter);                                         // off the layer

    layoutH->addWidget(slowButton);
    QPushButton *cleanButton = new QPushButton("Clean");
    connect (cleanButton, SIGNAL(clicked()), this, SLOT(clean()));
    layoutH->addWidget(cleanButton);
    layoutV->addLayout(layoutH);


    QWidget *bg = new QWidget();
    bg->setMinimumWidth(640);
    bg->setMinimumHeight(480);
    bg->setStyleSheet("QWidget { background-color: black; }");
    layoutV->addWidget(bg);


    fakeView = new FakeWindow(freej, (Layer *)NULL, &freej->screen->geo, bg);
    fakeView->setStyleSheet("QWidget { background-color: blue; }");
    SpecialEventGet* eventGet = new SpecialEventGet();
    fakeView->installEventFilter(eventGet);
    fakeView->setToolTip("Drag Left button to move Viewport, Drag center to resize");


    FakeWindow *fakeLay = new FakeWindow(freej, lay, &lay->geo, fakeView);
    fakeLay->setStyleSheet("QWidget { background-color: red; }");
    fakeLay->installEventFilter(eventGet);
    fakeLay->setToolTip("Drag Left button to move Layer, Drag center to resize");

    setLayout(layoutV);
}

// TextLayer
QqWidget::QqWidget(Context *freej, TextLayer *textLay) : QWidget()
{
    ctx = freej;
    setAttribute(Qt::WA_DeleteOnClose);
    isTextLayer = false;
    layerSet = false;
    newIdx = 0;

    addLayer(textLay);
    QVBoxLayout *layoutV = new QVBoxLayout;
    QHBoxLayout *layoutH = new QHBoxLayout;

    textButton = new QPushButton ("Apply");
    connect(textButton, SIGNAL(clicked()), this, SLOT(modTextLayer()));
    layoutH->addWidget(textButton);

    QComboBox *fontSizeBox = new QComboBox;
    for (int i=40;i>=5;i--)
    {
        fontSizeBox->insertItem(0, QString::number(i));
    }
    qTextLayer->set_fontsize(15);
    fontSizeBox->setCurrentIndex(10);
    connect(fontSizeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFontSize(int)));

    layoutH->addWidget(fontSizeBox);
    QqComboBlit *blt = new QqComboBlit;
    blt->addTextLayer(qTextLayer);

    layoutH->addWidget(blt);
    QqComboFilter *filter = new QqComboFilter(freej, textLay);
    layoutH->addWidget(filter);

    layoutV->addLayout(layoutH);
    QTextEdit *texto = new QTextEdit();
    text = texto;
    texto->setMaximumHeight(40);
    layoutV->addWidget(texto);

    QWidget *bg = new QWidget();
    bg->setMinimumWidth(640);
    bg->setMinimumHeight(480);
    bg->setStyleSheet("QWidget { background-color: black; }");
    layoutV->addWidget(bg);


    fakeView = new FakeWindow(freej, (Layer *)NULL, &freej->screen->geo, bg);
    fakeView->setStyleSheet("QWidget { background-color: blue; }");
    SpecialEventGet* eventGet = new SpecialEventGet();
    fakeView->installEventFilter(eventGet);
    fakeView->setToolTip("Drag Left button to move Viewport, Drag center to resize");

    FakeWindow *fakeLay = new FakeWindow(freej, textLay, &textLay->geo, fakeView);
    fakeLay->setStyleSheet("QWidget { background-color: red; }");
    fakeLay->installEventFilter(eventGet);
    fakeLay->setToolTip("Drag Left button to move Layer, Drag center to resize");

    setLayout(layoutV);
}

QqWidget::~QqWidget()
{
    Entry *le, *fe;

    bool res = false;

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
/*
    if (qTextLayer)
        //delete qTextLayer;
        qDebug() << "quit";
    else if (qLayer)
        delete qLayer;
*/
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
    char tmp[2048];

    QString txt = text->toPlainText();
    QByteArray fich = txt.toAscii();
    strcpy (tmp,fich.data());
    qTextLayer->write(tmp);
    cout << "text changed\n" << endl;
}

void QqWidget::slowDown()
{
    //normal speed devided by two
    if (!isTextLayer)
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

void QqWidget::addLayer(Layer *lay)
{
    isTextLayer = false;
    qLayer = lay;
    qTextLayer = NULL;
    layerSet = true;
}

void QqWidget::addLayer(TextLayer *lay)
{
    isTextLayer = true;
    qTextLayer = lay;
    qLayer = NULL;
    layerSet = true;
}
