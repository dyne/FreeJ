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


using namespace std;

QqTabWidget::QqTabWidget() : QTabWidget()
{
    setToolTip("you can change Layers order by sliding tabs");
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
}

QqTabWidget::~QqTabWidget()
{
    /*
int idx = this->count();
    while (idx)
    {
        if (QqWidget *widg = qobject_cast<QqWidget *>(this->widget(idx)))
        {
            qDebug() << "del layer";
            delete (widg->qLayer);
        }
        --idx;
    }
*/
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
        widg->qLayer->move(newFrom);
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

FakeWindow::FakeWindow(Context *context, Layer *layer, Geometry *geo, QWidget *parent) : QWidget()
{
    setParent(parent);
    qContext = context;
    qLayer = layer;
    qTextLayer = NULL;
    winGeo = new QRect(geo->x, geo->y, (int)geo->w/2, (int)geo->h/2);
    setGeometry(*winGeo);
    qDebug() << winGeo->x() << ":" << winGeo->y() << " " << winGeo->width() << ":" << winGeo->height();
}

FakeWindow::FakeWindow(Context *context, TextLayer *textlayer, Geometry *geo, QWidget *parent) : QWidget()
{
    setParent(parent);
    qContext = context;
    qTextLayer = textlayer;
    qLayer = NULL;
    winGeo = new QRect(geo->x, geo->y, (int)geo->w/2, (int)geo->h/2);
    setGeometry(*winGeo);
}

FakeWindow::~FakeWindow()
{
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
    layoutH->addWidget(filter);

    layoutH->addWidget(slowButton);

    QPushButton *resizeButton = new QPushButton("resize");              //for tests
    connect (resizeButton, SIGNAL(clicked()), this, SLOT(chgSize()));   //
    layoutH->addWidget(resizeButton);
    layoutV->addLayout(layoutH);

    QWidget *bg = new QWidget();
    bg->setMinimumWidth(640);
    bg->setMinimumHeight(480);
    bg->setStyleSheet("QWidget { background-color: black; }");
    layoutV->addWidget(bg);
//    SpecialEventGet* eventGet = new SpecialEventGet();    en attente
//    bg->installEventFilter(eventGet);                     en attente

    FakeWindow *fakeView = new FakeWindow(freej, lay, &freej->screen->geo, bg);   //(freej, lay, &freej->screen->geo);
    fakeView->setStyleSheet("QWidget { background-color: blue; }");
    fakeView->raise();



    setLayout(layoutV);
}

// TextLayer
QqWidget::QqWidget(Context *freej, QTextEdit *texto, TextLayer *textLay) : QWidget()
{
    ctx = freej;
    setAttribute(Qt::WA_DeleteOnClose);
    text = texto;
    isTextLayer = false;
    layerSet = false;
    newIdx = 0;

    addLayer((Layer *)textLay);
    addTextLayer(textLay);
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
    layoutV->addWidget(texto);

    setLayout(layoutV);
}

QqWidget::~QqWidget()
{
    if (isTextLayer && qTextLayer)
        delete qTextLayer;
    else if (!isTextLayer && qLayer)
        delete qLayer;
}

void QqWidget::chgSize()
{
    if (ctx && qLayer)
    {
        Geometry geo = qLayer->geo;
        qDebug() << "X:" << geo.x << " Y:" << geo.y << " W:" << geo.w << " H:" << geo.h << "zx:" << qLayer->zoom_x << " zy:" << qLayer->zoom_y;
        qLayer->set_zoom(0.5, 0.5);
        double x, y;
        x = (double)geo.w * 0.5;
        y = (double)geo.h * 0.5;
        double dec_x = -x / 2;
        double dec_y = -y / 2;
        qLayer->set_position((int)dec_x, (int)dec_y);
        ctx->cafudda(0.0);
        geo = qLayer->geo;
        qDebug() << "X:" << geo.x << " Y:" << geo.y << " W:" << geo.w << " H:" << geo.h << "zx:" << qLayer->zoom_x << " zy:" << qLayer->zoom_y;
    }
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
    // modifier le texte du bouton
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
    layerSet = true;
}

void QqWidget::addTextLayer(TextLayer *lay)
{
    isTextLayer = true;
    qTextLayer = lay;
    layerSet = true;
}
