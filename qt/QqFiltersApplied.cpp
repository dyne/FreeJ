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



#include <QqFiltersApplied.h>
#include <iostream>
#include <QWidget>
#include <QDragEnterEvent>
#include <QAction>
#include <QLabel>
#include <QDebug>
#include <QString>
#include <QByteArray>
#include <QSignalBlocker>
#include <QTimer>

using namespace std;

QqSlider::QqSlider(int idx, FilterInstance *filter, QWidget *parent) : QSlider(parent)
{
    paramNumber = idx;
    filterI = filter;
    setOrientation(Qt::Horizontal);
    setRange(0, 100);
    refreshValue();
    connect(this, &QSlider::valueChanged, this, &QqSlider::changeValue);
}

QqSlider::~QqSlider()
{
}

void QqSlider::changeValue(int value)
{
    double nbr = (double)value / 100.0;
    if (filterI->set_parameter_value(paramNumber, &nbr))
        if (auto *screen = filterI->get_layer()->screen)
            screen->clear();
}

void QqSlider::refreshValue()
{
    if (isSliderDown())
        return;
    std::lock_guard<Linklist<Parameter>> lock(filterI->parameters);
    auto *parameter = filterI->parameters.pick(paramNumber);
    const double stored = *static_cast<double *>(parameter->get());
    if (parameter->multiplier == 0.0)
        return;
    const QSignalBlocker block(this);
    setValue(qRound(100 * stored / parameter->multiplier));
}

QqCheck::QqCheck(int idx, FilterInstance *filter, QWidget *parent) : QCheckBox(parent)
{
    m_paramNumber = idx;
    m_filterI = filter;
    bool* val = (bool *) m_filterI->parameters[m_paramNumber]->get ();
    this->setDown(*val);
    connect(this, SIGNAL(stateChanged(int)), this, SLOT(changeValue(int)));
}

void QqCheck::changeValue(int state)
{
    bool bol;
    if (state)
    {
        bol = true;
        m_filterI->parameters[m_paramNumber]->set((void *)&bol);
    }
    else
    {
        bol = false;
        m_filterI->parameters[m_paramNumber]->set((void *)&bol);
    }
    m_filterI->get_layer()->screen->clear();  //cleans the screen
}

QqColor::QqColor(int idx, FilterInstance *filter, QWidget *parent) : QGroupBox(parent)
{
    m_paramNumber = idx;
    m_filterI = filter;
    QGridLayout *layout = new QGridLayout();
    this->setTitle(filter->parameters[idx]->name);

    double rgb[3];
//    double *rgb_init;
/*
    it seems that there is no default value, so don't use
    rgb_init = (double *) m_filterI->parameters[m_paramNumber]->get ();
    rgb[0] = rgb_init[0];
    rgb[1] = rgb_init[1];
    rgb[2] = rgb_init[2];
*/

    rgb[0] = 0.5;
    rgb[1] = 0.5;
    rgb[2] = 0.5;

    QSlider *red = new QSlider();
    red->setOrientation(Qt::Horizontal);
    connect(red, SIGNAL(sliderMoved(int)), this, SLOT(changeRed(int)));
    red->setRange(0, 255);
    int value = (int)(rgb[0] * 255.0);
    red->setValue(value);
    red->setSliderDown(true);
    layout->addWidget(red, 0, 0);

    QSlider *green = new QSlider();
    green->setOrientation(Qt::Horizontal);
    connect(green, SIGNAL(sliderMoved(int)), this, SLOT(changeGreen(int)));
    green->setSliderDown(true);
    green->setRange(0, 255);
    value = (int)(rgb[1] * 255.0);
    green->setValue(value);
    layout->addWidget(green, 1, 0);

    QSlider *blue = new QSlider();
    blue->setOrientation(Qt::Horizontal);
    connect(blue, SIGNAL(sliderMoved(int)), this, SLOT(changeBlue(int)));
    blue->setSliderDown(true);
    blue->setRange(0, 255);
    value = (int)(rgb[2] * 255.0);
    blue->setValue(value);
    layout->addWidget(blue, 2, 0);
    this->setLayout(layout);
 }

void QqColor::changeColor (int val, int idx)
{
    double rgb[3];
    double *rgb_init;
    rgb_init = (double *) m_filterI->parameters[m_paramNumber]->get ();
    rgb[0] = rgb_init[0];
    rgb[1] = rgb_init[1];
    rgb[2] = rgb_init[2];
    rgb[idx] = val/255.0;
    m_filterI->parameters[m_paramNumber]->set((void *)rgb);
    m_filterI->get_layer()->screen->clear();  //cleans the screen
}

void QqColor::changeRed(int val)
{
    changeColor (val,0);
}

void QqColor::changeGreen(int val)
{
    changeColor (val,1);
}

void QqColor::changeBlue(int val)
{
    changeColor (val,2);
}

QqPos::QqPos(int idx, FilterInstance *filter, QWidget *parent) : QGroupBox(parent)
{
    m_paramNumber = idx;
    m_filterI = filter;
    QGridLayout *layout = new QGridLayout();
    this->setTitle(filter->parameters[idx]->name);

    double pos[2];
//    double *pos_init;
/*
    it seems that there is no default value, so I don't use
    pos_init = (double *) m_filterI->parameters[m_paramNumber]->get ();
    pos[0] = pos_init[0];
    pos[1] = pos_init[1];
*/
    //default values
    pos[0] = 0.0;
    pos[1] = 0.0;

    QSlider *x = new QSlider(this);
    x->setOrientation(Qt::Horizontal);
    connect(x, SIGNAL(sliderMoved(int)), this, SLOT(changeX(int)));
    x->setRange(0, 1000);
    int value = (int)(pos[0] * 1000.0);
    x->setValue(value);
    x->setSliderDown(true);
    layout->addWidget(x, 0, 0);

    QSlider *y = new QSlider(this);
    y->setOrientation(Qt::Horizontal);
    connect(y, SIGNAL(sliderMoved(int)), this, SLOT(changeY(int)));
    y->setSliderDown(true);
    y->setRange(0, 1000);
    value = (int)(pos[1] * 1000.0);
    y->setValue(value);
    layout->addWidget(y, 1, 0);

    this->setLayout(layout);
 }

void QqPos::changePos (int val, int idx)
{
    double pos[2];
    double *pos_init;
    pos_init = (double *) m_filterI->parameters[m_paramNumber]->get ();
    pos[0] = pos_init[0];
    pos[1] = pos_init[1];
    pos[idx] = val/1000.0;
    m_filterI->parameters[m_paramNumber]->set((void *)pos);
    m_filterI->get_layer()->screen->clear();  //cleans the screen
}

void QqPos::changeX(int val)
{
    changePos (val,0);
}

void QqPos::changeY(int val)
{
    changePos (val,1);
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
  setAttribute(Qt::WA_QuitOnClose, false);
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
    auto *refresh = new QTimer(this);
    refresh->start(40);

    for (int i=1; i <= filterI->parameters.len(); ++i)
    {
        QHBoxLayout* layoutH = new QHBoxLayout;
        if (filterI->parameters[i]->type == Parameter::NUMBER)
        {
            name = new QLabel(filterI->parameters[i]->name);
            layoutH->addWidget(name);
            QqSlider *slider = new QqSlider(i, filterI, this);
            connect(refresh, &QTimer::timeout, slider, [slider] {
                if (slider->isVisible())
                    slider->refreshValue();
            });
            layoutH->addWidget(slider);
            layoutV->addLayout(layoutH);
            setLayout(layoutV);
            this->show();
            qDebug() << "NUMBER";
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
            QqPos *pos = new QqPos(i, filterI, this);
            layoutH->addWidget(pos);
            layoutV->addLayout(layoutH);
            this->show();

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

//layer
QqFiltersListApplied::QqFiltersListApplied(Layer *lay, QWidget* parent) : QListWidget(parent)
{
    setToolTip("Filters applied on the layer. You can change the order by D&D\nalso double click to hide parameters");
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);

    connect(this, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(setElement(QListWidgetItem*)));
    connect(this, &QListWidget::currentItemChanged, this, [lay](QListWidgetItem *item, QListWidgetItem *) {
        auto *filter = dynamic_cast<QqFilter *>(item);
        lay->filters.select_only(filter ? filter->instance() : nullptr);
    });
    draggItemFilter = NULL;
    layer = lay;

    QAction *remFilter= new QAction("remove filter",this);
    remFilter->setShortcut(Qt::Key_Delete);
    addAction(remFilter);
    connect(remFilter,SIGNAL(triggered()),this,SLOT(removeFilter()));
    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(showParamWindow(QListWidgetItem*)));
}

QqFiltersListApplied::QqFiltersListApplied(TextLayer *lay, QWidget *parent)
    : QqFiltersListApplied(static_cast<Layer *>(lay), parent)
{
}

QqFiltersListApplied::QqFiltersListApplied(GeneratorLayer *lay, QWidget *parent)
    : QqFiltersListApplied(static_cast<Layer *>(lay), parent)
{
}

QqFiltersListApplied::~QqFiltersListApplied()
{
    disconnect(this, nullptr, this, nullptr);
}

void QqFiltersListApplied::setElement(QListWidgetItem *item)
{
    draggItemFilter = dynamic_cast<QqFilter *>(item);
}

void QqFiltersListApplied::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void QqFiltersListApplied::dropEvent(QDropEvent *event)
{
    auto *dragged = draggItemFilter;
    draggItemFilter = nullptr;
    if (dragged) {
        const int destination = row(itemAt(event->pos())) + 1;
        if (destination < 1 || destination > layer->filters.len())
            return;
        dragged->instance()->move(destination);
    }
    QListWidget::dropEvent(event);
}

void QqFiltersListApplied::removeFilter()
{
    if (!hasFocus())
        return;
    if (auto *item = dynamic_cast<QqFilter *>(currentItem())) {
        item->instance()->rem();
        if (draggItemFilter == item)
            draggItemFilter = nullptr;
        delete item;
    }
}

void QqFiltersListApplied::showParamWindow(QListWidgetItem *item)
{
    auto *filter = dynamic_cast<QqFilter *>(item);
    if (!filter)
        return;
    if (filter->filterParam->isHidden())
    {
        filter->filterParam->setVisible(true);
        qDebug() << "double on ";
    }
    else
        filter->filterParam->close();
}
