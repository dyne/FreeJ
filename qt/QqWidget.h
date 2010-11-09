#ifndef QQWIDGET_H
#define QQWIDGET_H
#include <QWidget>
#include <text_layer.h>
#include <QTabWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <context.h>
#include <qfreej.h>
#include <QBoxLayout>

class Qfreej;

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

private:
    TextLayer *qTextLayer;
    QRect *winGeo;
    Context *qContext;
    Layer *qLayer;
    FakeWindow* fakeView;
};

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


class QqWidget : public QWidget
{
    Q_OBJECT
public:
    QqWidget();
    QqWidget(Context *,  QqTabWidget*, Qfreej*, QString);   //Layer
    QqWidget(Context *, QqTabWidget*, Qfreej*);             //TextLayer
    ~QqWidget();
    FakeWindow* getFake();
    Layer* getLayer();
    TextLayer* getTextLayer();
    Context* getContext();


public slots:
    void slowDown();
    void modTextLayer();
    void changeFontSize(int);
    void clean();

private:
    int slowFps;
    int normalFps;
    int actualFps;
    int newIdx;
    Layer *qLayer;
    TextLayer *qTextLayer;
    QTextEdit *text;
    QPushButton *textButton;
    QPushButton *slowButton;
    QComboBox *fontSizeBox;
    Context *ctx;
    FakeWindow* fakeView;
    QVBoxLayout* layoutV;
    QHBoxLayout* layoutH;
};
#endif // QQWIDGET_H
