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
    QStringList filterStrlist;
    QDir dirFrei0r = QDir("/usr/lib/frei0r-1");
    if (!dirFrei0r.exists())
        dirFrei0r.setPath("/usr/local/lib/frei0r-1");
    else if (!dirFrei0r.exists())
        dirFrei0r.setPath("/usr/lib64/frei0r-1");
    else if (!dirFrei0r.exists())
        dirFrei0r.setPath("/opt/local/lib/frei0r-1");
    else if (!dirFrei0r.exists())
    {
        QMessageBox::information(this, "frei0r", "unable to find frei0r lib directory");
        return;
    }

    QStringList fileFilter;
    fileFilter << "*.so";
    dirFrei0r.setNameFilters(fileFilter);
    cout << "frei0r number filter : " << dirFrei0r.count() << endl;
    QFileInfoList filt = dirFrei0r.entryInfoList();
    QByteArray fich;
    Freior *fr;
    char temp[256];


    for (int i=0; i < filt.size(); ++i)
    {
        fr = (Freior *)Factory<Filter>::new_instance("Frei0rFilter");
        fich = filt.at(i).fileName().toAscii();
        strcpy (temp,fich.data());
        if( !fr || !fr->open(temp) ) {
          delete fr;
        } else { // freior effect found
            // check what kind of plugin is and place it
            if(fr->info.plugin_type == F0R_PLUGIN_TYPE_FILTER && filt.at(i).baseName() != "3dflippo") {
                filterStrlist << filt.at(i).baseName();
//                cout << "ajout d'un filtre : " << filt.at(i).baseName().toStdString() << endl;
                delete fr;
            }
        }
    }
/*
    filterStrlist << "3dflippo" << "B" << "G" << "R" << "Brightness" << "bw0r" << "Cartoon";
    filterStrlist << "K-Means Clustering" <<  "Color Distance" <<  "Contrast0r" << "delay0r";
    filterStrlist << "Distort0r" << "Edgeglow" << "Equaliz0r" << "Flippo" << "Gamma" << "Glow";
    filterStrlist << "Hueshift0r" << "Invert0r" << "Lens Correction" << "LetterB0xed" << "Luminance";
    filterStrlist << "Mask0Mate" << "nosync0r" << "Perspective" << "pixeliz0r" << "RGB-Parade";
    filterStrlist << "Saturat0r" << "Scale0Tilt" << "scanline0r" << "Sobel" << "Squareblur";
    filterStrlist << "TehRoxx0r" << "Threshold0r" << "Transparency" << "Twolay0r" << "Vectorscope";
    filterStrlist << "Water";
*/
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
    qTextLayer = lay;
    QStringList filterStrlist;
    char temp[256];

    QDir dirFrei0r = QDir("/usr/lib/frei0r-1");
    if (!dirFrei0r.exists())
        dirFrei0r.setPath("/usr/local/lib/frei0r-1");
    else if (!dirFrei0r.exists())
        dirFrei0r.setPath("/usr/lib64/frei0r-1");
    else if (!dirFrei0r.exists())
        dirFrei0r.setPath("/opt/local/lib/frei0r-1");
    else if (!dirFrei0r.exists())
    {
        QMessageBox::information(this, "frei0r", "unable to find frei0r lib directory");
        return;
    }
    QStringList fileFilter;
    fileFilter << "*.so";
    dirFrei0r.setNameFilters(fileFilter);
    cout << "frei0r number filter : " << dirFrei0r.count() << endl;
    QFileInfoList filt = dirFrei0r.entryInfoList();

    QByteArray fich;
    Freior *fr;

    for (int i=0; i < filt.size(); ++i)
    {
        fr = (Freior *)Factory<Filter>::new_instance("Frei0rFilter");
        fich = filt.at(i).fileName().toAscii();
        strcpy (temp,fich.data());
        if( !fr || !fr->open(temp) ) {
          delete fr;
        } else { // freior effect found
            // check what kind of plugin is and place it
            if(fr->info.plugin_type == F0R_PLUGIN_TYPE_FILTER && filt.at(i).baseName() != "3dflippo") {
                filterStrlist << filt.at(i).baseName();
//                cout << "ajout d'un filtre : " << filt.at(i).baseName().toStdString() << endl;
                delete fr;
            }
        }
    }
/*
    filterStrlist << "3dflippo" << "B" << "G" << "R" << "Brightness" << "bw0r" << "Cartoon";
    filterStrlist << "K-Means Clustering" <<  "Color Distance" <<  "Contrast0r" << "delay0r";
    filterStrlist << "Distort0r" << "Edgeglow" << "Equaliz0r" << "Flippo" << "Gamma" << "Glow";
    filterStrlist << "Hueshift0r" << "Invert0r" << "Lens Correction" << "LetterB0xed" << "Luminance";
    filterStrlist << "Mask0Mate" << "nosync0r" << "Perspective" << "pixeliz0r" << "RGB-Parade";
    filterStrlist << "Saturat0r" << "Scale0Tilt" << "scanline0r" << "Sobel" << "Squareblur";
    filterStrlist << "TehRoxx0r" << "Threshold0r" << "Transparency" << "Twolay0r" << "Vectorscope";
    filterStrlist << "Water";
*/
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
    QqFilter *filter = new QqFilter(filterI);
    filtersListApplied->addItem(filter);
}

void QqComboFilter::addLayer(Layer *lay)
{
    qLayer = lay;
}

void QqComboFilter::addTextLayer(TextLayer *lay)
{
    qTextLayer = lay;
}
