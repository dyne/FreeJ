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


QqComboFilter::QqComboFilter(Context *context, Layer *layer, QWidget *parent)
    : QWidget(parent), layer(layer), freej(context)
{
    auto *filterBox = new QComboBox(this);
    for (auto **available = freej->filters.completion(""); *available; ++available)
        filterBox->addItem((*available)->name);
    filterBox->setEnabled(filterBox->count() != 0);
    connect(filterBox, SIGNAL(activated(QString)), this, SLOT(addFilter(QString)));

    auto *layout = new QHBoxLayout(this);
    layout->addWidget(filterBox);
    filtersListApplied = new QqFiltersListApplied(layer, this);
    layout->addWidget(filtersListApplied);
}

QqComboFilter::QqComboFilter(Context *context, TextLayer *layer, QWidget *parent)
    : QqComboFilter(context, static_cast<Layer *>(layer), parent)
{
}

QqComboFilter::QqComboFilter(Context *context, GeneratorLayer *layer, QWidget *parent)
    : QqComboFilter(context, static_cast<Layer *>(layer), parent)
{
}

void QqComboFilter::chgParam(double)
{
}

void QqComboFilter::addFilter(QString name)
{
    const QByteArray filterName = name.toLatin1();
    auto *prototype = freej->filters.search(filterName.constData());
    if (!prototype) {
        qWarning() << "filter not found:" << name;
        return;
    }
    auto *instance = prototype->apply(layer);
    if (!instance) {
        qWarning() << "error applying filter" << name << "on layer" << layer->name;
        return;
    }
    filtersListApplied->addItem(new QqFilter(instance));
}
