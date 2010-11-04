#ifndef SPECIALEVENTGET_H
#define SPECIALEVENTGET_H

#include <QObject>
#include <QPoint>

class SpecialEventGet : public QObject
{
protected:
    bool eventFilter(QObject*, QEvent*);
    Qt::MouseButton btn;
    QPoint offset;

};

#endif // SPECIALEVENTGET_H
