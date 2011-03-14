#ifndef SPECIALEVENTGET_H
#define SPECIALEVENTGET_H

#include <QObject>
#include <QPoint>
#include <QSize>
#include <layer.h>

class SpecialEventGet : public QObject
{
public:
    SpecialEventGet(QObject*);
    void setShift();

protected:
    bool eventFilter(QObject*, QEvent*);
private:
    Qt::MouseButton btn;
    QPoint offset, shift;
    QSize m_OldSize;
    long diff;		//tracks the mouse x position from starting point
};

#endif // SPECIALEVENTGET_H
