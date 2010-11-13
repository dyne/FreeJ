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


#include <QqComboFilter.h>
#include <QqWidget.h>
#include <iostream>
#include <QBoxLayout>
#include <QqFiltersApplied.h>
#include <QDir>
#include <frei0r_freej.h>
#include <QDebug>
#include <QMessageBox>

using namespace std;


QqComboFilter::QqComboFilter(Context *freejQ, Layer *lay, QWidget* parent) : QWidget(parent)
{
    freej = freejQ;
    qLayer = lay;
    qTextLayer = NULL;
    m_qGeneLayer = NULL;
    QStringList filterStrlist;

    Filter **res;
    Filter *filt;
    int c;

    res = freej->filters.completion((char*)"");
    if(!res[0]) {
      qDebug() << "no generators found";
      return;
    }
    for(c=0;res[c];c++) {
        filt = res[c];
        if(!filt) break;
        filterStrlist << filt->name;
    }

    filterBox = new QComboBox(this);
    filterBox->view()->dragEnabled();
    filterBox->addItems(filterStrlist);
    connect(filterBox, SIGNAL(activated(QString)), this, SLOT(addFilter(QString)));
    layoutH = new QHBoxLayout;
    layoutH->addWidget(filterBox);
    filtersListApplied = new QqFiltersListApplied(lay, this);

    layoutH->addWidget(filtersListApplied);
    this->setLayout(layoutH);
}

QqComboFilter::QqComboFilter(Context *freejQ, TextLayer *lay, QWidget *parent) : QWidget(parent)
{
    freej = freejQ;
    qLayer = NULL;
    m_qGeneLayer = NULL;
    qTextLayer = lay;
    QStringList filterStrlist;

    Filter **res;
    Filter *filt;
    int c;

    res = freej->filters.completion((char*)"");
    if(!res[0]) {
      qDebug() << "no generators found";
      return;
    }
    for(c=0;res[c];c++) {
        filt = res[c];
        if(!filt) break;
        filterStrlist << filt->name;
    }

    filterBox = new QComboBox(this);
    filterBox->view()->dragEnabled();
    filterBox->addItems(filterStrlist);
    connect(filterBox, SIGNAL(activated(QString)), this, SLOT(addFilter(QString)));
    layoutH = new QHBoxLayout;
    layoutH->addWidget(filterBox);

    filtersListApplied = new QqFiltersListApplied(lay, this);
    layoutH->addWidget(filtersListApplied);
    this->setLayout(layoutH);
}

QqComboFilter::QqComboFilter(Context *freejQ, GeneratorLayer *lay, QWidget *parent) : QWidget(parent)
{
    freej = freejQ;
    qLayer = NULL;
    qTextLayer = NULL;
    m_qGeneLayer = lay;
    QStringList filterStrlist;

    Filter **res;
    Filter *filt;
    int c;

    res = freej->filters.completion((char*)"");
    if(!res[0]) {
      qDebug() << "no generators found";
      return;
    }
    for(c=0;res[c];c++) {
        filt = res[c];
        if(!filt) break;
        filterStrlist << filt->name;
    }

    filterBox = new QComboBox(this);
    filterBox->view()->dragEnabled();
    filterBox->addItems(filterStrlist);
    connect(filterBox, SIGNAL(activated(QString)), this, SLOT(addFilter(QString)));
    layoutH = new QHBoxLayout;
    layoutH->addWidget(filterBox);

    filtersListApplied = new QqFiltersListApplied(lay, this);
    layoutH->addWidget(filtersListApplied);
    this->setLayout(layoutH);
}


QqComboFilter::~QqComboFilter()
{
    delete layoutH;
}

void QqComboFilter::chgParam(double prm)
{
}

void QqComboFilter::addFilter(QString flt)
{
    char filterName[256];
    QByteArray folt = flt.toAscii();
    Filter *filt;
    int idx;

    strcpy (filterName,folt.data());

    filt = (Filter*)freej->filters.search(filterName, &idx);
    if(!filt) {
       cout << "filter not found: " << flt.toStdString() << endl;
       return;
    }

    FilterInstance *filterI;
    if (qTextLayer)
    {
        filterI = filt->apply(qTextLayer);
        if( !filterI ) {
            cout << "error applying filter " << flt.toStdString() << " on layer " << qTextLayer->name << endl;
            return;
        }
    }
    else if (qLayer)
    {
        filterI = filt->apply(qLayer);
        if( !filterI) {
            cout << "error applying filter " << flt.toStdString() << " on layer " << qLayer->name << endl;
            return;
        }
    }
    else if (m_qGeneLayer)
    {
        filterI = filt->apply(m_qGeneLayer);
        if( !filterI) {
            cout << "error applying filter " << flt.toStdString() << " on layer " << m_qGeneLayer->name << endl;
            return;
        }
    }
    QqFilter *filter = new QqFilter(filterI);
    filtersListApplied->addItem(filter);
}
/*
void QqComboFilter::addLayer(Layer *lay)
{
    qLayer = lay;
}

void QqComboFilter::addTextLayer(TextLayer *lay)
{
    qTextLayer = lay;
}
*/
