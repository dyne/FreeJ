#ifndef QQCOMBOBLIT_H
#define QQCOMBOBLIT_H

#include <QComboBox>
#include <layer.h>
#include <text_layer.h>
#include <QDoubleSpinBox>
#include <QBoxLayout>
#include <generator_layer.h>

class QqComboBlit : public QWidget
{
    Q_OBJECT
public:
    QqComboBlit(QWidget*);
    ~QqComboBlit();

    void addLayer(Layer *);
    void addTextLayer(TextLayer *);
    void addGeneLayer(GeneratorLayer *);

public slots:
    void changeBlit(QString);
    void chgParam(double);   // à finir

private :
    //bool isTextLayer;
    //bool layerSet;
    Layer *qLayer;
    TextLayer *qTextLayer;
    GeneratorLayer *m_qGeneLayer;
    QComboBox *blitBox;
    QDoubleSpinBox *blitParam;
    QHBoxLayout* layoutH;
};

#endif // QQCOMBOBLIT_H
