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


#include "specialeventget.h"
#include <QMouseEvent>
#include <QDebug>
#include <QWidget>
#include <QqWidget.h>
#include <QString>

extern QSize viewSize;

SpecialEventGet::SpecialEventGet(QObject* parent) : QObject(parent)
{
    shift.setX(0);
    shift.setY(0);
    offset.setX(0);
    offset.setY(0);
}

bool SpecialEventGet::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::Move) {
        QMoveEvent* evt = static_cast<QMoveEvent*>(event);
    } else if(event->type() == QEvent::Resize) {
        QResizeEvent* evt = static_cast<QResizeEvent*>(event);
        m_OldSize = QSize(evt->oldSize().width(), evt->oldSize().height());
    } else if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* evt = static_cast<QMouseEvent*>(event);
        btn= evt->button();
        offset = evt->pos();
    } else if(event->type() == QEvent::MouseMove)
    {
        QMouseEvent* evt = static_cast<QMouseEvent*>(event);
        FakeWindow *fake = static_cast<FakeWindow*>(obj);
        Context* context = fake->getContext();
        Layer* layer = fake->getLayer();
        TextLayer* textlayer = fake->getTextLayer();
        if (btn==Qt::LeftButton)  //change position, only layers
        {
            if (layer)
            {
                fake->move(fake->mapToParent(evt->pos() - offset));
                layer->set_position ((fake->pos().x() + shift.x()), (fake->pos().y()+shift.y()));
                context->screen->clear();
                //qDebug() << "posX:" << fake->pos().x() << "shiftX:" << shift.x();
            }
            else if (textlayer)
            {
                fake->move(fake->mapToParent(evt->pos() - offset));
                textlayer->set_position ((fake->pos().x() + shift.x()), (fake->pos().y()+shift.y()));
                context->screen->clear();
            }
            // todo textlayer dans QqWidget
        }
        else if (btn==Qt::RightButton && layer) //resize the layer
        {
            int sizeW = fake->size().width() + evt->pos().x() - offset.x();
            int sizeH = fake->size().height() + evt->pos().y() - offset.y();
            fake->resize(sizeW, sizeH);
            offset = evt->pos();
            double x1, y1;
            x1 = m_OldSize.width() * 1.0 / layer->geo.w;
            y1 = m_OldSize.height() * 1.0 / layer->geo.h;
            layer->set_zoom(x1, y1);
            shift.setX((int)((m_OldSize.width() - layer->geo.w)/2));
            shift.setY((int)((m_OldSize.height() - layer->geo.h)/2));
            layer->set_position ((shift.x()+fake->pos().x()), (shift.y()+fake->pos().y()));
            context->screen->clear();
        }
        else if (btn==Qt::RightButton && textlayer) //resize textlayer
        {
            int sizeW = fake->size().width() + evt->pos().x() - offset.x();
            int sizeH = fake->size().height() + evt->pos().y() - offset.y();
            fake->resize(sizeW, sizeH);
            offset = evt->pos();
            double x1, y1;
            x1 = m_OldSize.width() * 1.0 / textlayer->geo.w;
            y1 = m_OldSize.height() * 1.0 / textlayer->geo.h;
            textlayer->set_zoom(x1, y1);
            shift.setX((int)((m_OldSize.width() - textlayer->geo.w)/2));
            shift.setY((int)((m_OldSize.height() - textlayer->geo.h)/2));
            textlayer->set_position ((shift.x()+fake->pos().x()), (shift.y()+fake->pos().y()));
            context->screen->clear();
        }
        else if (btn==Qt::RightButton)  //resize viewport
        {
            int sizeW = fake->size().width() + evt->pos().x() - offset.x();
            int sizeH = fake->size().height() + evt->pos().y() - offset.y();
            fake->resize(sizeW, sizeH);
            viewSize.setWidth(sizeW);
            viewSize.setHeight(sizeH);
            context->screen->resize(sizeW, sizeH);
            offset = evt->pos();
            context->screen->clear();

            //to refresh layer dispay problems
            int c = context->screen->layers.len();
            QString blitName;
            for (;c > 0; c--)
            {
                Layer* lay = context->screen->layers[c];
                blitName = lay->get_blit();
                if (blitName != "RGB" && blitName != "SDL" && blitName!="SRCALPHA")
                {
                    lay->set_blit("SDL");
                    lay->set_blit(blitName.toStdString().c_str());
                }
            }
        }
        else if (btn==Qt::MidButton) // was MiddleButton
        {
            if (layer)
            {
                if (evt->pos().x() - offset.x() > 0)
                {
                    int angle = fake->getAngle() + (evt->pos().x() - offset.x()) / 10 % 360;
                    if (angle >= 360)
                        angle = 0;
                    qDebug() << "angle:" << angle;
                    fake->setAngle(angle);
                }
                else if (evt->pos().x() - offset.x() < 0)
                {
                    int angle = fake->getAngle() + (evt->pos().x() - offset.x()) / 10 % 360;
                    if (angle <= -360)
                        angle = 0;
                    qDebug() << "angle:" << angle;
                    fake->setAngle(angle);
                }
                fake->repaint();
                layer->set_rotate(- fake->getAngle());
                context->screen->clear();
            }
            else if (textlayer)
            {
                if (evt->pos().x() - offset.x() > 0)
                {
                    int angle = fake->getAngle() + (evt->pos().x() - offset.x()) / 10 % 360;
                    if (angle >= 360)
                        angle = 0;
                    fake->setAngle(angle);
                }
                else if (evt->pos().x() - offset.x() < 0)
                {
                    int angle = fake->getAngle() + (evt->pos().x() - offset.x()) / 10 % 360;
                    if (angle <= -360)
                        angle = 0;
                    fake->setAngle(angle);
                }
                fake->repaint();
                textlayer->set_rotate(- fake->getAngle());
                context->screen->clear();
            }
        }
    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        offset = QPoint();
    }
    else if(event->type() == QEvent::Paint)
    {
        FakeWindow *fake = static_cast<FakeWindow*>(obj);
        if (fake->getLayer() || fake->getTextLayer())
        {
            QqWidget *widg = qobject_cast<QqWidget *>(this->parent());
            if (widg)
            {
                widg->setAngle((double)fake->getAngle());
            }
            QPainter* painter = fake->getPainter();
            if (painter->begin(fake))
            {
                painter->save();

                QColor textColor(255, 255, 255);
                painter->setPen(textColor);
                if (fake->getTextLayer())
                {
                    QString text;
                    text = text.sprintf ("zX %01.03g zY %01.03g %4dx%4d"
                                         , fake->getTextLayer()->zoom_x, fake->getTextLayer()->zoom_y
                                         , fake->geometry().width(), fake->geometry().height());
                    QPoint pos(10, 10);
                    painter->drawText(pos, text);
                }
                else if (fake->getLayer())
                {
                    QString text;
                    text = text.sprintf ("zX %01.03g Zy %01.03g %4dx%4d"
                                         , fake->getLayer()->zoom_x, fake->getLayer()->zoom_y
                                         , fake->geometry().width(), fake->geometry().height());
                    QPoint pos(10, 10);
                    painter->drawText(pos, text);
                }

                QColor color(127, 127, 127, 191);
                QRect size(fake->geometry());
                size.setTopLeft(QPoint(0,0));
                QPoint center = size.center();
                painter->translate(center.x(), center.y());
                painter->setPen(Qt::NoPen);
                painter->setBrush(color);
                painter->rotate(fake->getAngle());
                painter->setRenderHint(QPainter::Antialiasing);
                size = QRect(-center.x(), -center.y(), size.width(), size.height());
                painter->drawRect(size);
                painter->restore();
                painter->end();
            }
        }
    }
    return true;
}

void SpecialEventGet::setShift()
{
    shift.setX(0);
    shift.setY(0);
}

