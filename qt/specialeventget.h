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
};

#endif // SPECIALEVENTGET_H
