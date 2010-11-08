#include <QqFiltersApplied.h>
#include <iostream>
#include <QWidget>
#include <QDragEnterEvent>
#include <QAction>
#include <QLabel>
#include <QDebug>
#include <QString>
#include <QByteArray>

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

QqCheck::QqCheck(int idx, FilterInstance *filter, QWidget *parent) : QCheckBox(parent)
{
    m_paramNumber = idx;
    m_filterI = filter;
    connect(this, SIGNAL(stateChanged(int)), this, SLOT(changeValue(int)));
}

void QqCheck::changeValue(int state)
{
    bool bol;
    if (state)
    {
        bol = true;
        m_filterI->parameters[m_paramNumber]->set((void *)&bol);
        qDebug() << "bool: " << m_paramNumber << ": true";
    }
    else
    {
        bol = false;
        m_filterI->parameters[m_paramNumber]->set((void *)&bol);
        qDebug() << "bool: " << m_paramNumber << ": false";
    }
}

QqColor::QqColor(int idx, FilterInstance *filter, QWidget *parent) : QGroupBox(parent)
{
    m_paramNumber = idx;
    m_filterI = filter;
    QGridLayout *layout = new QGridLayout();
    this->setTitle(filter->parameters[idx]->name);

    QSlider *red = new QSlider();
    red->setOrientation(Qt::Horizontal);
    connect(red, SIGNAL(sliderMoved(int)), this, SLOT(changeRed(int)));
    red->setRange(0, 1000);
    red->setSliderDown(true);
    layout->addWidget(red, 0, 0);

    QSlider *green = new QSlider();
    green->setOrientation(Qt::Horizontal);
    connect(green, SIGNAL(sliderMoved(int)), this, SLOT(changeGreen(int)));
    green->setSliderDown(true);
    green->setRange(1, 255);
    layout->addWidget(green, 1, 0);

    QSlider *blue = new QSlider();
    blue->setOrientation(Qt::Horizontal);
    connect(blue, SIGNAL(sliderMoved(int)), this, SLOT(changeBlue(int)));
    blue->setSliderDown(true);
    blue->setRange(1, 255);
    layout->addWidget(blue, 2, 0);
    this->setLayout(layout);
 }

void QqColor::changeRed(int val)
{
    double* r = NULL;

    if (m_filterI->parameters[m_paramNumber]->value_size == 24)
    {
        r = (double *)m_filterI->parameters[m_paramNumber]->value;
        *r = val/100.0;
        m_filterI->parameters[m_paramNumber]->set((void *)r);
        qDebug() << "r : " << *r;
    }
}

void QqColor::changeGreen(int val)
{
    double* r = NULL;
    double* g = NULL;

    if (m_filterI->parameters[m_paramNumber]->value_size == 24)
    {
        r = (double*)m_filterI->parameters[m_paramNumber]->value;
        g = r + 8;
        *g = val/100.0;
        m_filterI->parameters[m_paramNumber]->set((void *)g);
        qDebug() << "g : " << *g;
    }
}

void QqColor::changeBlue(int val)
{
    double* r = NULL;
    double* b = NULL;

    if (m_filterI->parameters[m_paramNumber]->value_size == 24)
    {
        r = (double*)m_filterI->parameters[m_paramNumber]->value;
        b = r + 16;
        *b = val/100.0;
        m_filterI->parameters[m_paramNumber]->set((void *)b);
        qDebug() << "b : " << *b;
    }
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
    QLabel *name;
    Layer* lay = filterI->get_layer();
    QString layS = lay->get_filename();
    QString title;
    title = filterI->name;
    title.append(" ");
    if (layS.count() > 1)
    {
        title.append(lay->get_filename());
    }
    else
    {
        title.append("text");
    }
    setWindowTitle(title);

    for (int i=1; i <= filterI->parameters.len(); ++i)
    {
        QHBoxLayout* layoutH = new QHBoxLayout;
        if (filterI->parameters[i]->type == Parameter::NUMBER)
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
        else if (filterI->parameters[i]->type == Parameter::BOOL)
        {
            name = new QLabel(filterI->parameters[i]->name);
            layoutH->addWidget(name);
            QqCheck *check = new QqCheck(i, filterI, this);
            check->setChecked(false);
            layoutH->addWidget(check);
            layoutV->addLayout(layoutH);
            setLayout(layoutV);
            this->show();
        }
        else if (filterI->parameters[i]->type == Parameter::COLOR)
        {
            QqColor *color = new QqColor(i, filterI, this);
            layoutH->addWidget(color);
            layoutV->addLayout(layoutH);
            this->show();
        }
        else if (filterI->parameters[i]->type == Parameter::POSITION)
        {
            qDebug() << "size POSITION:" << filterI->parameters[i]->value_size;
        }
        else if (filterI->parameters[i]->type == Parameter::STRING)
        {
            qDebug() << "size STRING:" << filterI->parameters[i]->value_size;
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
