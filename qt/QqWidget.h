#ifndef QQWIDGET_H
#define QQWIDGET_H
#include <QWidget>
#include <text_layer.h>
#include <QTabWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <context.h>

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
};


class QqWidget : public QWidget
{
    Q_OBJECT
public:
    QqWidget();
    QqWidget(Context *, QTextEdit *, TextLayer *);
    QqWidget(Context *, Layer *);
    ~QqWidget();
    void addLayer(Layer *);
    void addTextLayer(TextLayer *);

    int slowFps;
    int normalFps;
    int actualFps;

    Layer *qLayer;

public slots:
    void slowDown();
    void modTextLayer();
    void changeFontSize(int);

private:
    int newIdx;
    TextLayer *qTextLayer;
    bool isTextLayer;
    bool layerSet;
    QTextEdit *text;
    QPushButton *textButton;
    QPushButton *slowButton;
    QComboBox *fontSizeBox;
};
#endif // QQWIDGET_H
