#include <QqComboBlit.h>
#include <QqWidget.h>
#include <iostream>
#include <QBoxLayout>
#include <QDebug>

using namespace std;


QqComboBlit::QqComboBlit() : QWidget()
{
    isTextLayer = false;
    layerSet = false;
    qLayer = NULL;
    qTextLayer = NULL;
    QStringList blits;
    blits << "RGB" << "ADD" << "SUB" << "MEAN" << "ABSDIFF MULT" << "MULTNOR DIV";
    blits << "MULTDIV2" << "MULTDIV4" << "AND" << "OR" << "XOR" << "RED" << "GREEN" << "BLUE";
    blits << "REDMASK" << "GREENMASK" << "BLUEMASK" << "NEG" << "ADDB" << "ADDBH" << "SUBB" << "SHL";
    blits << "SHLB" << "SHR" << "MULB" << "BIN" << "SDL" << "ALPHA" << "SRCALPHA";
    blitBox = new QComboBox;
    blitBox->addItems(blits);
    connect(blitBox, SIGNAL(activated(QString)), this, SLOT(addBlit(QString)));
    QHBoxLayout *layoutH = new QHBoxLayout;
    layoutH->addWidget(blitBox);
    blitParam = new QDoubleSpinBox;
    connect(blitParam, SIGNAL(valueChanged(double)), this, SLOT(chgParam(double)));
    blitParam->setDecimals(1);
    blitParam->setMinimum(0);
    blitParam->setMaximum(256);
    blitParam->setSingleStep(0.1);
    blitParam->setEnabled(false);
    layoutH->addWidget(blitParam);
    this->setLayout(layoutH);
}

QqComboBlit::~QqComboBlit()
{
}

void QqComboBlit::chgParam(double prm)
{
    Blit *blit;
    if (isTextLayer)
    {
        blit = qTextLayer->current_blit;
    }
    else
    {
        blit = qLayer->current_blit;
    }
    blit->value = blitParam->value();
    qDebug() << blit->value;
}

void QqComboBlit::addBlit(QString blt)
{
    char blitName[256];
    QByteArray bolt = blt.toAscii();

    strcpy (blitName,bolt.data());

    if (!layerSet)
    {
        cout << "QqComboBox Layer not set" << endl;
        return;
    }
    blitParam->setEnabled(false);

    if (isTextLayer)
    {
        Blit *blit = qTextLayer->current_blit;
        qTextLayer->set_blit(blitName);
        blit = qTextLayer->current_blit;
        cout << "blit : " << blit->desc << endl;    //think about add it in a text zone
//        cout << "blit name : " << blit->name << " parameters len : " << blit->parameters.len() << endl;
        if (blit->parameters.len())
        {
            blitParam->setEnabled(true);
        }
    }
    else
    {
        Blit *blit = qLayer->current_blit;
        qLayer->set_blit(blitName);
        blit = qLayer->current_blit;
        cout << "blit : " << blit->desc << endl;
//        cout << "blit name : " << blit->name << " parameters len : " << blit->parameters.len() << endl;
        if (blit->parameters.len())
        {
            blitParam->setEnabled(true);
        }
    }
}

void QqComboBlit::addLayer(Layer *lay)
{
    isTextLayer = false;
    qLayer = lay;
    layerSet = true;
}

void QqComboBlit::addTextLayer(TextLayer *lay)
{
    isTextLayer = true;
    qTextLayer = lay;
    layerSet = true;
}
