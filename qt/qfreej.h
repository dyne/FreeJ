#ifndef QFREEJ_H
#define QFREEJ_H
#include <QMessageBox>

#include <QMainWindow>
#include <text_layer.h>
#include <context.h>
#include <linklist.h>
#include <QMdiArea>
#include <QqWidget.h>
#include <QLayout>

class QTimer;

namespace Ui {
    class Qfreej;
}

class Qfreej : public QWidget/*QMainWindow*/ {
    Q_OBJECT
public:
    Qfreej(QWidget *parent = 0);
    ~Qfreej();

    bool _isPlaying;
    QTimer *poller;
    Context *getContext();

public slots:
    void addLayer();
    void updateInterface();
    void addTextLayer();
    void startStreaming();

//    friend class QqLayer; non necessaire semble-t-il


protected:
    //void changeEvent(QEvent *e);//old

private:
    //Ui::Qfreej *ui;//old
    void init();
    Context *freej;
    ViewPort *screen;
    bool startstate;
    int fps;
    QqTabWidget *tabWidget;
    QTabBar *myTabBar;
    int number;
    TextLayer *textLayer;
    QGridLayout *grid;//new
};



#endif // QFREEJ_H
