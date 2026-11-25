#ifndef QQCOMBOFILTER_H
#define QQCOMBOFILTER_H
#include <QComboBox>
#include <QDoubleSpinBox>
#include <context.h>
#include <text_layer.h>
#include <QListWidget>
#include <QqFiltersApplied.h>
#include <generator_layer.h>

class QqComboFilter : public QWidget
{
    Q_OBJECT
public:
    QqComboFilter(Context *, Layer *, QWidget*);
    QqComboFilter(Context *, TextLayer *, QWidget*);
    QqComboFilter(Context *, GeneratorLayer *, QWidget*);
public slots:
    void addFilter(QString);
    void chgParam(double);   // à finir

private :
    Layer *layer;
    QqFiltersListApplied *filtersListApplied;
    Context *freej;
};
#endif // QQCOMBOFILTER_H
