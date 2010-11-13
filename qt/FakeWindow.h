#ifndef FAKEWINDOW_H
#define FAKEWINDOW_H

#include <QWidget>
#include <text_layer.h>
#include <QPainter>
#include <context.h>
#include <specialeventget.h>
#include <generator_layer.h>

class FakeWindow : public QWidget
{
public:
    FakeWindow(Context *, Layer*, Geometry*, QWidget*);
    FakeWindow(Context *, TextLayer*, Geometry*, QWidget*);
    FakeWindow(Context *, GeneratorLayer*, Geometry*, QWidget*);
    ~FakeWindow();
    Context *getContext();
    Layer *getLayer();
    TextLayer *getTextLayer();
    GeneratorLayer *getGeneLayer();
    QRect* getWinGeo();
    int getAngle();
    void setAngle(int);
    QPainter *getPainter();
    void resetZoom();
    void setEventGet(SpecialEventGet*);

private:
    QRect *winGeo;
    Context *m_ctx;
    Layer *qLayer;
    TextLayer *qTextLayer;
    GeneratorLayer *m_qGeneLayer;
    FakeWindow* fakeView;
    int m_angle;
    QPainter* m_painter;
    SpecialEventGet* m_eventGet;
};

#endif // FAKEWINDOW_H
