#ifndef QQFILTERSAPPLIED_H
#define QQFILTERSAPPLIED_H

#include <QListWidget>
#include <filter.h>
#include <text_layer.h>
#include <QKeyEvent>

class QqSlider : public QSlider
{
    Q_OBJECT
public:
    QqSlider(int, FilterInstance *, QWidget *);
    ~QqSlider();

public slots:
    void changeValue(int);

private:
    int paramNumber;
    FilterInstance *filterI;
};

class QqFilterParam : public QWidget
{
    Q_OBJECT
public:
    QqFilterParam(FilterInstance *);
    ~QqFilterParam();

private:
    FilterInstance *filterI;
};

class QqFilter : public QListWidgetItem
{
public:
    QqFilter(FilterInstance *);
    ~QqFilter();
    QqFilterParam *filterParam;

private:
    FilterInstance *filterIn;
};

class QqFiltersListApplied : public QListWidget
{
    Q_OBJECT
public:
    QqFiltersListApplied(Layer *);
    QqFiltersListApplied(TextLayer *);
    ~QqFiltersListApplied();

public slots:
    void setElement (QListWidgetItem *);
    void removeFilter();
    void showParamWindow(QListWidgetItem *);

protected:
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);
    void inputKeyPressEvent(QKeyEvent*);

private:
    Layer *layer;
    TextLayer *textLayer;
    QqFilter *draggItemFilter;
};

#endif // QQFILTERSAPPLIED_H
