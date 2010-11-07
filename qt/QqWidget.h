#ifndef QQWIDGET_H
#define QQWIDGET_H
#include <QWidget>
#include <text_layer.h>
#include <QTabWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <context.h>

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
    QqTabWidget();
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
    QqWidget(Context *, TextLayer *);
    QqWidget(Context *, Layer *);
    ~QqWidget();
    void addLayer(Layer *);
    void addLayer(TextLayer *);
    FakeWindow* getFake();
    Layer* getLayer();
    TextLayer* getTextLayer();

    int slowFps;
    int normalFps;
    int actualFps;

public slots:
    void slowDown();
    void modTextLayer();
    void changeFontSize(int);
    void clean();

private:
    int newIdx;
    Layer *qLayer;
    TextLayer *qTextLayer;
    bool isTextLayer;
    bool layerSet;
    QTextEdit *text;
    QPushButton *textButton;
    QPushButton *slowButton;
    QComboBox *fontSizeBox;
    Context *ctx;
    FakeWindow* fakeView;
};
#endif // QQWIDGET_H
