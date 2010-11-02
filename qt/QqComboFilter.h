#ifndef QQCOMBOFILTER_H
#define QQCOMBOFILTER_H
#include <QComboBox>
#include <QDoubleSpinBox>
#include <context.h>
#include <text_layer.h>
#include <QListWidget>
#include <QqFiltersApplied.h>

class QqComboFilter : public QWidget
{
    Q_OBJECT
public:
    QqComboFilter(Context *, Layer *);
    QqComboFilter(Context *, TextLayer *);
    ~QqComboFilter();

    void addLayer(Layer *);
    void addTextLayer(TextLayer *);

public slots:
    void addFilter(QString);
    void chgParam(double);   // à finir

private :
    bool isTextLayer;
    bool layerSet;
    Layer *qLayer;
    TextLayer *qTextLayer;
    QComboBox *filterBox;
    QDoubleSpinBox *filterParam;
    QqFiltersListApplied *filtersListApplied;
    Context *freej;
};
#endif // QQCOMBOFILTER_H
