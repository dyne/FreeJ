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


using namespace std;

QqTabWidget::QqTabWidget() : QTabWidget()
{
    setToolTip("you can change Layers order by sliding tabs");
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
}

QqTabWidget::~QqTabWidget()
{
    int idx = this->count();
    while (idx)
    {
        if (QqWidget *widg = qobject_cast<QqWidget *>(this->widget(idx)))
        {
            qDebug() << "del : " << widg->qLayer->name;
            delete (widg->qLayer);
        }
        --idx;
    }
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
        delete (widg->qLayer);
    }
    removeTab(idx);
}

//Layer
QqWidget::QqWidget(Context *freej, Layer *lay) : QWidget()
{
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
    layoutV->addLayout(layoutH);

    QGridLayout *grid = new QGridLayout();
    QPushButton *viewPortWindow = new QPushButton("ViewPort");
    grid->addWidget(viewPortWindow);
    QPushButton *sizeWindow = new QPushButton(lay->get_filename());
    sizeWindow->setFixedSize(300, 200);
    viewPortWindow->setFixedSize(400, 300);
    grid->addWidget(sizeWindow);
    layoutV->addLayout(grid);

    setLayout(layoutV);
}

// TextLayer
QqWidget::QqWidget(Context *freej, QTextEdit *texto, TextLayer *textLay) : QWidget()
{
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
