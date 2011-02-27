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
#include <Sound.h>
#include "qOpacity.h"
#include "qJackClient.h"

class QTimer;
class QqTabWidget;
class QOpacity;

namespace Ui {
    class Qfreej;
}

class Qfreej : public QWidget {
    Q_OBJECT
public:
    Qfreej(QWidget *parent = 0);
    ~Qfreej();

    QTimer *poller;
    Context *getContext();
    bool getStartState();
    void createMenuGenerator();
    bool IsAudioOn();
    QJackClient *m_QJack;

public slots:
    void addLayer();
    void updateInterface();
    void addTextLayer();
    void startStreaming();
    void addGenerator(QAction*);
    void openSoundDevice();
    void changeOpacity();
    void jackConnect();

protected:
    void closeEvent(QCloseEvent*);

private:
    void init();
    Context *freej;
    ViewPort *screen;
    bool startstate;
    int fps;
    QqTabWidget *tabWidget;
    QTabBar *myTabBar;
    int number;
    TextLayer *textLayer;   //see to erase this later
    QGridLayout *grid;
    QHBoxLayout *layoutH;
    QMenu* menuGenerator;
    Sound *m_snd;
    QOpacity *m_Opacity;
    bool m_JackIsOn;
};



#endif // QFREEJ_H
