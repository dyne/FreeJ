#ifndef FAKEWINDOW_H
#define FAKEWINDOW_H

#include <QWidget>
#include <text_layer.h>
#include <QPainter>

class FakeWindow : public QWidget
{
public:
    FakeWindow(Context *, Layer*, Geometry*, QWidget*);
    FakeWindow(Context *, TextLayer*, Geometry*, QWidget*);
    ~FakeWindow();
    Context *getContext();
    Layer *getLayer();
    TextLayer *getTextLayer();
    QRect* getWinGeo();
    int getAngle();
    void setAngle(int);
    QPainter *getPainter();

private:
    TextLayer *qTextLayer;
    QRect *winGeo;
    Context *qContext;
    Layer *qLayer;
    FakeWindow* fakeView;
    int m_angle;
    QPainter* m_painter;
};

#endif // FAKEWINDOW_H
