#include "qfreej.h"
#include "ui_qfreej.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <string>
#include <iostream>
#include <QList>
#include <QqWidget.h>
#include <QTimer>
#include <QDebug>

using namespace std;

Qfreej::Qfreej(QWidget *parent) :
        QWidget(parent)
{

    _isPlaying = true;

    grid = new QGridLayout(this);

    QMenuBar *menuBar = new QMenuBar(this);
    grid->addWidget(menuBar, 0, 0);
    QMenu *menuFichier = menuBar->addMenu("&Fichier");

    QAction *actionPen = menuFichier->addAction("&Open");
    poller = new QTimer (this);

    connect(actionPen, SIGNAL(triggered()), this, SLOT(addLayer()));

    connect(poller, SIGNAL(timeout()), this, SLOT(updateInterface()));

    poller->start(10); //start timer to trigger every 10 ms the updateInterface slot

    freej = new Context();
    init();
    number = 0;
    setAttribute(Qt::WA_DeleteOnClose);
}

Qfreej::~Qfreej()
{
    delete freej;

    poller->stop();
}

void Qfreej::init()
{
    QHBoxLayout *layoutH = new QHBoxLayout();//new
    grid->addLayout(layoutH, 1, 0);//new
    QPushButton *streamButton = new QPushButton("stream", this);//new
    layoutH->addWidget(streamButton);//new
    connect(streamButton, SIGNAL(clicked()), this, SLOT(startStreaming()));//new

    tabWidget = new QqTabWidget;
    grid->addWidget(tabWidget, 2, 0);//new
    setLayout(grid);//new
    tabWidget->setMovable(true);
    tabWidget->setAttribute(Qt::WA_DeleteOnClose);
    tabWidget->setTabsClosable(true);

    myTabBar = tabWidget->getTabBar();
    myTabBar->setTabsClosable(true);
    myTabBar->setAttribute(Qt::WA_DeleteOnClose);
    connect(myTabBar, SIGNAL(tabMoved(int, int)), tabWidget, SLOT(moveLayer(int, int)));

    startstate = true;
    fps = 25;                   //fps par défaut
    screen = Factory<ViewPort>::get_instance( "Screen", "sdl" );
    screen->init(640, 480, 32);
    freej->add_screen(screen);
    freej->config_check("keyboard.js");
    freej->interactive = false;
    freej->fps.set( fps );
    freej->start_running = startstate;
    addTextLayer();

    cout << "ViewPort largeur : " << screen->geo.w << " hauteur : " << screen->geo.h << endl;
}

void Qfreej::startStreaming()
{
    QString fichier = QFileDialog::getOpenFileName(this, "Ouvrir un fichier JavaScript", QString(), "All (*.js)");
    char temp[256];
    QByteArray fich = fichier.toAscii();
    strcpy (temp,fich.data());
    freej->open_script(temp);
    std::cout << temp;
}

void Qfreej::addLayer()
{
    QString fichier = QFileDialog::getOpenFileName(this, "Ouvrir un fichier", QString(), "All (*.*)");

    char temp[256];
    QByteArray fich = fichier.toAscii();
    strcpy (temp,fich.data());

    Layer *lay = NULL;
    lay = freej->open(temp, 0, 0); // hey, this already init and open the layer !!

    if(lay)  {
        if( screen->add_layer(lay) ) {
            lay->start();
            cout << "largeur : " << lay->geo.w << " hauteur : " << lay->geo.h << endl;
            cout << "ViewPort largeur : " << screen->geo.w << " hauteur : " << screen->geo.h << endl;

            cout << "nom du layer : " << lay->get_filename() << endl;

            QqWidget *aWidget = new QqWidget(freej, lay);
            if (lay->frame_rate > 50)   //pb de determination de FPS
            {
                lay->fps.set(lay->frame_rate / 10);
                aWidget->normalFps = lay->frame_rate / 10;
                aWidget->actualFps = lay->frame_rate / 10;

            }
            else
            {
                lay->fps.set(lay->frame_rate);
                aWidget->normalFps = lay->frame_rate;
                aWidget->actualFps = lay->frame_rate;
            }
            aWidget->slowFps = aWidget->normalFps / 2;
            tabWidget->addTab(aWidget, lay->get_filename());
        }
        else
        {
            QMessageBox::information(this, "Layers", "Impossible d'ouvrir : " + fichier + " (layer)\n");
            if (!startstate)
                lay->active = false;
        }
    }
}

void Qfreej::addTextLayer()
{
    textLayer = (TextLayer *)freej->open((char *)"textouille.txt", 0, 0);
    if (textLayer)
    {
        textLayer->init(640, 100, 32);
        textLayer->set_position(0, 380);
        textLayer->set_font("sans");
        if( screen->add_layer(textLayer) )
        {
            textLayer->start();
            textLayer->fps.set(fps);

            QqWidget *aWidget = new QqWidget(freej, textLayer);
            tabWidget->addTab(aWidget, "text zone");
        }
        else
        {
            QMessageBox::information(this, "Layers", "Impossible de créer la TextLayer\n");
            if (!startstate)
              textLayer->active = false;
        }
    }
    else
    {
        QMessageBox::information(this, "Layers", "Impossible de créer la TextLayer\n");
    }
}


void Qfreej::updateInterface()
{
    if (!_isPlaying)
        return;

    freej->cafudda(0.0);
}


Context *Qfreej::getContext()
{
    return (freej);
}

/*old
void Qfreej::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
*/
