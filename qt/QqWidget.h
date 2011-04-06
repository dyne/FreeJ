#ifndef QQWIDGET_H
#define QQWIDGET_H
#include <QWidget>
#include <text_layer.h>
#include <video_layer.h>
//#include <v4l2_layer.h>
#include <QTabWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <context.h>
#include <qfreej.h>
#include <QBoxLayout>
#include <QDoubleSpinBox>
#include <QPainter>
#include <FakeWindow.h>
#include <QqTabWidget.h>
#include <generator_layer.h>

class Qfreej;


class QqWidget : public QWidget
{
    Q_OBJECT
public:
    QqWidget();
    QqWidget(Context *,  QqTabWidget*, Qfreej*, QString);   //Layer
    QqWidget(Context *, QqTabWidget*, Qfreej*);             //TextLayer
    QqWidget(Context *, QqTabWidget*, Qfreej*, QAction*);   //Geometry
    ~QqWidget();
    FakeWindow* getFake();
    Layer* getLayer();
    TextLayer* getTextLayer();
    GeneratorLayer* getGeneLayer();
    Context* getContext();
    void setAngle(double);
    double getAngle();
    QqTabWidget* getTabWidget();

public slots:
    void changeFps(int);
    void modTextLayer();
    void changeFontSize(int);
    void clean();
    void changeAngle(double);
    void redrawFake();
    void playPause();
    void resetZoom();

private:
    int newIdx;
    Layer *qLayer;
    TextLayer *qTextLayer;
    GeneratorLayer *m_qGeneLayer;
    QTextEdit *text;
    QPushButton *textButton;
    QPushButton *slowButton;
    QSlider *slowFps;
    float normalFps;
    float actualFps;
    int	fpsP;
    QPushButton *playButton;
    bool isPlaying;
    QComboBox *fontSizeBox;
    Context *ctx;
    FakeWindow* fakeView;
    FakeWindow* fakeLay;
    QVBoxLayout* layoutV;
    QHBoxLayout* layoutH;
    QDoubleSpinBox *m_angleBox;
    double m_angle;
    QqTabWidget *m_tabWidg;
};
#endif // QQWIDGET_H
