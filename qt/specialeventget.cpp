#include "specialeventget.h"
#include <QMouseEvent>
#include <QDebug>
#include <QWidget>
#include <QqWidget.h>

SpecialEventGet::SpecialEventGet() : QObject()
{
    shift.setX(0);
    shift.setY(0);
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
        if (btn==Qt::LeftButton)
        {
            if (layer)
            {
                fake->move(fake->mapToParent(evt->pos() - offset));
                layer->set_position ((fake->pos().x() + shift.x()), (fake->pos().y()+shift.y()));
                //qDebug() << "posX:" << fake->pos().x() << "shiftX:" << shift.x();
            }
            else if (textlayer)
            {
                fake->move(fake->mapToParent(evt->pos() - offset));
                textlayer->set_position ((fake->pos().x() + shift.x()), (fake->pos().y()+shift.y()));
            }
            // todo textlayer dans QqWidget
        }
        else if (btn==Qt::RightButton && layer)
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
            //qDebug() << "zx:" << x1 << "sX:" << shift.x() << "posX:" << fake->pos().x();
        }
        else if (btn==Qt::RightButton && textlayer)
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
            //qDebug() << "zx:" << x1 << "sX:" << shift.x() << "posX:" << fake->pos().x();
        }
        else if (btn==Qt::RightButton)
        {
            int sizeW = fake->size().width() + evt->pos().x() - offset.x();
            int sizeH = fake->size().height() + evt->pos().y() - offset.y();
            fake->resize(sizeW, sizeH);
            context->screen->resize(sizeW, sizeH);
            offset = evt->pos();
        }
    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        offset = QPoint();
    }
    return true;
}
