#ifndef QQFILTERSAPPLIED_H
#define QQFILTERSAPPLIED_H

#include <QListWidget>
#include <filter.h>
#include <text_layer.h>
#include <QKeyEvent>
#include <QCheckBox>
#include <QBoxLayout>
#include <QGroupBox>
#include <generator_layer.h>


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

class QqCheck : public QCheckBox
{
    Q_OBJECT
public:
    QqCheck(int, FilterInstance *, QWidget *);

public slots:
    void changeValue(int);

private:
    int m_paramNumber;
    FilterInstance *m_filterI;
};

class QqColor : public QGroupBox
{
    Q_OBJECT
public:
    QqColor(int, FilterInstance *, QWidget *);

public slots:
    void changeRed(int);
    void changeGreen(int);
    void changeBlue(int);

private:
    void changeColor(int, int);
    int m_paramNumber;
    FilterInstance *m_filterI;
};

class QqPos : public QGroupBox
{
    Q_OBJECT
public:
    QqPos(int, FilterInstance *, QWidget *);

public slots:
    void changeX(int);
    void changeY(int);

private:
    void changePos(int, int);
    int m_paramNumber;
    FilterInstance *m_filterI;
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
    QqFiltersListApplied(Layer *, QWidget*);
    QqFiltersListApplied(TextLayer *, QWidget*);
    QqFiltersListApplied(GeneratorLayer *, QWidget*);
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
    GeneratorLayer *m_geneLayer;
    QqFilter *draggItemFilter;
};

#endif // QQFILTERSAPPLIED_H
