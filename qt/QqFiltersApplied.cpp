#include <QqFiltersApplied.h>
#include <iostream>
#include <QBoxLayout>
#include <QWidget>
#include <QDragEnterEvent>
#include <QAction>
#include <QLabel>
#include <QDebug>

using namespace std;

QqSlider::QqSlider(int idx, FilterInstance *filter, QWidget *parent) : QSlider()
{
    paramNumber = idx;
    filterI = filter;
    setOrientation(Qt::Horizontal);
    connect(this, SIGNAL(sliderMoved(int)), this, SLOT(changeValue(int)));
}

QqSlider::~QqSlider()
{
}

void QqSlider::changeValue(int value)
{
    double nbr = (double)value / 100.0;
    //to see if it does something ... and it does :)
    filterI->parameters[paramNumber]->set((void *)&nbr);
    qDebug() << "nbr: " << paramNumber << ": " << nbr;
}


QqFilter::QqFilter(FilterInstance *filterI) : QListWidgetItem()
{
    setText(filterI->name);
    filterIn = filterI;
    filterParam = new QqFilterParam(filterI);
}

QqFilter::~QqFilter()
{
    delete filterParam;
}

QqFilterParam::QqFilterParam(FilterInstance *filter) : QWidget()
{
    filterI = filter;
    QVBoxLayout *layoutV = new QVBoxLayout;
    QHBoxLayout *layoutH;
    QLabel *name;
    for (int i=1; i <= filterI->parameters.len(); ++i)
    {
        setWindowTitle(filterI->name);
        layoutH = new QHBoxLayout;
        if (filterI->parameters[i]->type == 1)
        {
            name = new QLabel(filterI->parameters[i]->name);
            layoutH->addWidget(name);
            QqSlider *slider = new QqSlider(i, filterI, this);
            slider->setSliderDown(true);
            slider->setRange(0,100);
            slider->setTickInterval(1);
            layoutH->addWidget(slider);
            layoutV->addLayout(layoutH);
            setLayout(layoutV);
            this->show();
        }
    }
}

QqFilterParam::~QqFilterParam()
{
}

QqFiltersListApplied::QqFiltersListApplied(Layer *lay) : QListWidget()
{
    setToolTip("Filters applied on the layer. You can change the order by D&D\nalso double click to hide parameters");
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);

    connect(this, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(setElement(QListWidgetItem*)));
    draggItemFilter = NULL;
    layer = lay;
    textLayer = NULL;

    QAction *remFilter= new QAction("remove filter",this);
    remFilter->setShortcut(Qt::Key_Delete);
    addAction(remFilter);
    connect(remFilter,SIGNAL(triggered()),this,SLOT(removeFilter()));
    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(showParamWindow(QListWidgetItem*)));
}

QqFiltersListApplied::QqFiltersListApplied(TextLayer *lay) : QListWidget()
{
    setToolTip("Filters applied on the layer. You can change the order by D&D\nalso double click to hide parameters");
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);

    connect(this, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(setElement(QListWidgetItem*)));
    draggItemFilter = NULL;
    layer = NULL;
    textLayer = lay;

    QAction *remFilter= new QAction("remove filter",this);
    remFilter->setShortcut(Qt::Key_Delete);
    addAction(remFilter);
    connect(remFilter,SIGNAL(triggered()),this,SLOT(removeFilter()));
    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(showParamWindow(QListWidgetItem*)));
}

QqFiltersListApplied::~QqFiltersListApplied()
{
}

void QqFiltersListApplied::setElement(QListWidgetItem *item)
{
    draggItemFilter = (QqFilter *)item;
}

void QqFiltersListApplied::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void QqFiltersListApplied::dropEvent(QDropEvent * event)
{
    if (draggItemFilter)
    {
        char temp[256];
        QByteArray fich = draggItemFilter->text().toAscii();
        strcpy (temp,fich.data());
        int idx = 0;
        int currentDropRow = row(itemAt(event->pos()));

        currentDropRow++;
        if (layer)
        {
            FilterInstance *filter = layer->filters.search(temp, &idx);
            if (currentDropRow >= 1 && currentDropRow <= layer->filters.len())
            {
                filter->move(currentDropRow);
            }
            else
            {
                draggItemFilter = NULL;
                return;
            }
        }
        else if (textLayer)
        {
            FilterInstance *filter = textLayer->filters.search(temp, &idx);
            if (currentDropRow >= 1 && currentDropRow <= textLayer->filters.len())
            {
                filter->move(currentDropRow);
            }
            else
            {
                draggItemFilter = NULL;
                return;
            }
        }
        else
        {
            cout << "pas de filtre à déplacer ?????" << endl;
            draggItemFilter = NULL;
            return;
        }
    }
    draggItemFilter = NULL;
    QListWidget::dropEvent(event);  // pour ne pas manger le signal
}

void QqFiltersListApplied::removeFilter()
{
    if (hasFocus() && count() >= 1 && currentRow() >=0)
    {
        int idx;
        QqFilter *item = (QqFilter *)currentItem();
        char temp[256];
        QByteArray fich = item->text().toAscii();
        strcpy (temp,fich.data());
        if (layer)
        {
            FilterInstance *filter = layer->filters.search(temp, &idx);
            delete item;
            filter->rem();
        }
        else if (textLayer)
        {
            FilterInstance *filter = textLayer->filters.search(temp, &idx);
            delete item;
            filter->rem();
        }
        else
        {
            cout << "Erreur : Impossible de supprimer le filtre : " << item->text().toStdString() << endl;
            return;
        }
    }
}

void QqFiltersListApplied::showParamWindow(QListWidgetItem *item)
{
    QqFilter *filter = (QqFilter *)item;
    if (filter->filterParam->isHidden())
    {
        filter->filterParam->setVisible(true);
        qDebug() << "double on ";
    }
    else
        filter->filterParam->close();
}
