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


#include <QqComboBlit.h>
#include <QqWidget.h>
#include <iostream>
#include <QBoxLayout>
#include <QDebug>

using namespace std;


QqComboBlit::QqComboBlit(QWidget* parent) : QWidget(parent)
{
    //isTextLayer = false;
    //layerSet = false;
    qLayer = NULL;
    qTextLayer = NULL;
    m_qGeneLayer = NULL;
    QStringList blits;
    blits << "RGB" << "ADD" << "SUB" << "MEAN" << "ABSDIFF MULT" << "MULTNOR DIV";
    blits << "MULTDIV2" << "MULTDIV4" << "AND" << "OR" << "XOR" << "RED" << "GREEN" << "BLUE";
    blits << "REDMASK" << "GREENMASK" << "BLUEMASK" << "NEG" << "ADDB" << "ADDBH" << "SUBB" << "SHL";
    blits << "SHLB" << "SHR" << "MULB" << "BIN" << "SDL" << "ALPHA" << "SRCALPHA";
    blitBox = new QComboBox(this);
    blitBox->setToolTip("BLIT applied to the layer");
    blitBox->addItems(blits);
    connect(blitBox, SIGNAL(activated(QString)), this, SLOT(addBlit(QString)));
    layoutH = new QHBoxLayout;
    layoutH->addWidget(blitBox);
    blitParam = new QDoubleSpinBox(this);
    blitParam->setToolTip("BLIT parameter value");
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
    delete layoutH;
}

void QqComboBlit::chgParam(double prm)
{
    Blit *blit;
    if (qTextLayer)
    {
        blit = qTextLayer->current_blit;
    }
    else if (qLayer)
    {
        blit = qLayer->current_blit;
    }
    else if (m_qGeneLayer)
    {
        blit = m_qGeneLayer->current_blit;
    }
    blit->value = blitParam->value();
}

void QqComboBlit::addBlit(QString blt)
{
    char blitName[256];
    QByteArray bolt = blt.toAscii();

    strcpy (blitName,bolt.data());

/*    if (!layerSet)
    {
        cout << "QqComboBox Layer not set" << endl;
        return;
    }
*/
    blitParam->setEnabled(false);

    if (qTextLayer)
    {
        Blit *blit = qTextLayer->current_blit;
        qTextLayer->set_blit(blitName);
        blit = qTextLayer->current_blit;
        if (blit->parameters.len())
        {
            blitParam->setEnabled(true);
        }
    }
    else if (qLayer)
    {
        Blit *blit = qLayer->current_blit;
        qLayer->set_blit(blitName);
        blit = qLayer->current_blit;
        if (blit->parameters.len())
        {
            blitParam->setEnabled(true);
        }
    }
    else if (m_qGeneLayer)
    {
        Blit *blit = m_qGeneLayer->current_blit;
        m_qGeneLayer->set_blit(blitName);
        blit = m_qGeneLayer->current_blit;
        if (blit->parameters.len())
        {
            blitParam->setEnabled(true);
        }
    }
}

void QqComboBlit::addLayer(Layer *lay)
{
    qLayer = lay;
}

void QqComboBlit::addTextLayer(TextLayer *lay)
{
    qTextLayer = lay;
}

void QqComboBlit::addGeneLayer(GeneratorLayer *lay)
{
    m_qGeneLayer = lay;
}

