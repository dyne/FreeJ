#ifndef QQTABWIDGET_H
#define QQTABWIDGET_H

#include <QTabWidget>
#include <QqWidget.h>

class QqTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    QqTabWidget(QWidget*);
    ~QqTabWidget();
    QTabBar *getTabBar();

public slots:
    void moveLayer (int, int);
    void closeTab (int);
    void setSize(int);
};



#endif // QQTABWIDGET_H
