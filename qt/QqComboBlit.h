#ifndef QQCOMBOBLIT_H
#define QQCOMBOBLIT_H

#include <QComboBox>
#include <layer.h>
#include <text_layer.h>
#include <QDoubleSpinBox>

class QqComboBlit : public QWidget
{
    Q_OBJECT
public:
    QqComboBlit();
    ~QqComboBlit();

    void addLayer(Layer *);
    void addTextLayer(TextLayer *);

public slots:
    void addBlit(QString);
    void chgParam(double);   // à finir

private :
    bool isTextLayer;
    bool layerSet;
    Layer *qLayer;
    TextLayer *qTextLayer;
    QComboBox *blitBox;
    QDoubleSpinBox *blitParam;
};

#endif // QQCOMBOBLIT_H
