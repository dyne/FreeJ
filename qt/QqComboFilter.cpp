
#include <QqComboFilter.h>
#include <QqWidget.h>
#include <iostream>
#include <QBoxLayout>
#include <QqFiltersApplied.h>
#include <QDir>
#include <frei0r_freej.h>
#include <QDebug>

using namespace std;


QqComboFilter::QqComboFilter(Context *freejQ, Layer *lay) : QWidget()
{
    freej = freejQ;
    isTextLayer = false;
    layerSet = true;
    qLayer = lay;
    qTextLayer = NULL;
    QStringList filterStrlist;
    QDir dirFrei0r = QDir("/usr/lib/frei0r-1");
    cout << "frei0r number filter : " << dirFrei0r.count() << endl;
    QStringList fileFilter;
    fileFilter << "*.so";
    dirFrei0r.setNameFilters(fileFilter);
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
    filterBox = new QComboBox;
    filterBox->view()->dragEnabled();
    filterBox->addItems(filterStrlist);
    connect(filterBox, SIGNAL(activated(QString)), this, SLOT(addFilter(QString)));
    QHBoxLayout *layoutH = new QHBoxLayout;
    layoutH->addWidget(filterBox);
    filtersListApplied = new QqFiltersListApplied(lay);

    layoutH->addWidget(filtersListApplied);
    this->setLayout(layoutH);
}

QqComboFilter::QqComboFilter(Context *freejQ, TextLayer *lay) : QWidget()
{
    freej = freejQ;
    isTextLayer = true;
    layerSet = true;
    qLayer = NULL;
    qTextLayer = lay;
    QStringList filterStrlist;
    char temp[256];

    QDir dirFrei0r = QDir("/usr/lib/frei0r-1");
    QStringList fileFilter;
    fileFilter << "*.so";
    dirFrei0r.setNameFilters(fileFilter);
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
    filterBox = new QComboBox;
    filterBox->view()->dragEnabled();
    filterBox->addItems(filterStrlist);
    connect(filterBox, SIGNAL(activated(QString)), this, SLOT(addFilter(QString)));
    QHBoxLayout *layoutH = new QHBoxLayout;
    layoutH->addWidget(filterBox);

    filtersListApplied = new QqFiltersListApplied(lay);
    layoutH->addWidget(filtersListApplied);
    this->setLayout(layoutH);
}


QqComboFilter::~QqComboFilter()
{
}

void QqComboFilter::chgParam(double prm)
{
    if (isTextLayer)
    {
    }
    else
    {
    }
}

void QqComboFilter::addFilter(QString flt)
{
    char filterName[256];
    QByteArray folt = flt.toAscii();
    Filter *filt;
    int idx;

    strcpy (filterName,folt.data());

    if (!layerSet)
    {
        cout << "QqComboBox Layer not set" << endl;
        return;
    }

    filt = (Filter*)freej->filters.search(filterName, &idx);
    if(!filt) {
       cout << "filter not found: " << flt.toStdString() << endl;
       return;
    }

    FilterInstance *filterI;
    if (isTextLayer)
    {
        filterI = filt->apply(qTextLayer);
        if( !filterI ) {
            cout << "error applying filter " << flt.toStdString() << " on layer " << qTextLayer->name << endl;
            return;
        }
    }
    else
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
    isTextLayer = false;
    qLayer = lay;
    layerSet = true;
}

void QqComboFilter::addTextLayer(TextLayer *lay)
{
    isTextLayer = true;
    qTextLayer = lay;
    layerSet = true;
}
