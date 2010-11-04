#include "specialeventget.h"
#include <QMouseEvent>
#include <QDebug>
#include <QWidget>

bool SpecialEventGet::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::Move) {
        QMoveEvent* evt = static_cast<QMoveEvent*>(event);
        qDebug() << "Pos\tx:" << evt->pos().x() << "\ty:" << evt->pos().y();
    } else if(event->type() == QEvent::Resize) {
        QResizeEvent* evt = static_cast<QResizeEvent*>(event);
        qDebug() << "Taille\tx:" << evt->size().width() << "\ty:" << evt->size().height();
    } else if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* evt = static_cast<QMouseEvent*>(event);
        btn= evt->button();
        offset = evt->pos();
    } else if(event->type() == QEvent::MouseMove)
    {
        QMouseEvent* evt = static_cast<QMouseEvent*>(event);
        QWidget *wid = static_cast<QWidget*>(obj);
        if (btn==Qt::LeftButton)
        {
                wid->move(wid->mapToParent(evt->pos() - offset));
        }
        else if (btn==Qt::MiddleButton)
        {
                wid->resize(wid->size().width() + evt->pos().x() - offset.x(),
                  wid->size().height() + evt->pos().y() - offset.y());
                offset = evt->pos();
        }

    } else if(event->type() == QEvent::MouseButtonRelease)
    {
        offset = QPoint();
    }
        return true;
}
