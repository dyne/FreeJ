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



#include <QqTabWidget.h>

extern QSize viewSize;


QqTabWidget::QqTabWidget(QWidget* parent) : QTabWidget(parent)
{
    parent->setToolTip("you can change Layers order by sliding tabs");
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(setSize(int)));
}

QqTabWidget::~QqTabWidget()
{
}

QTabBar *QqTabWidget::getTabBar()
{
    return (tabBar());
}

void QqTabWidget::moveLayer(int from, int to)
{
    if (QqWidget *widg = qobject_cast<QqWidget *>(this->widget(from)))
    {
        Context* ctx = widg->getContext();
        int newFrom = from;
        newFrom++;

        if (Layer* lay = widg->getLayer())
            lay->move(newFrom);
        else if (TextLayer* textlay = widg->getTextLayer())
            textlay->move(newFrom);
        ctx->screen->clear();
    }
}

void QqTabWidget::closeTab(int idx)
{
    if (QqWidget *widg = qobject_cast<QqWidget *>(this->widget(idx)))
    {
        widg->close();
    }
    removeTab(idx);
  }

//keeps all fake viewport the same size.
void QqTabWidget::setSize(int idx)
{
    if (idx >= 0)
    {
        QqWidget* widg = qobject_cast<QqWidget *>(this->widget(idx));
        if (widg)
        {
            FakeWindow* fake = widg->getFake();
            if (fake)
                fake->resize(viewSize);
        }
    }
}

