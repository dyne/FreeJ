/*  Qfreej
 *  (c) Copyright 2010 fred_99
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */



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
#include <QCloseEvent>

using namespace std;
extern QSize viewSize;

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
    poller->stop();
/*
    while (freej->screen->layers.len() > 0)
    {
        freej->screen->layers[1]->rem();
    }
*/
//    delete freej;
    delete layoutH;
    delete grid;
}

void Qfreej::closeEvent(QCloseEvent *event)
{
    qDebug() << "je sors !!";
    event->accept();
    delete freej;
}

void Qfreej::init()
{
    layoutH = new QHBoxLayout();//new
    grid->addLayout(layoutH, 1, 0);//new
    QPushButton *streamButton = new QPushButton("stream", this);//new
    layoutH->addWidget(streamButton);//new
    connect(streamButton, SIGNAL(clicked()), this, SLOT(startStreaming()));//new

    tabWidget = new QqTabWidget(this);
    grid->addWidget(tabWidget, 2, 0);//new
    setLayout(grid);//new
    tabWidget->setMovable(true);
    //tabWidget->setAttribute(Qt::WA_DeleteOnClose);
    tabWidget->setTabsClosable(true);

    myTabBar = tabWidget->getTabBar();
    myTabBar->setTabsClosable(true);
    myTabBar->setAttribute(Qt::WA_DeleteOnClose);
    connect(myTabBar, SIGNAL(tabMoved(int, int)), tabWidget, SLOT(moveLayer(int, int)));

    startstate = true;
    fps = 25;                   //fps par défaut
    screen = Factory<ViewPort>::get_instance( "Screen", "sdl" );
    screen->init(viewSize.width(), viewSize.height(), 32);
    freej->add_screen(screen);
    freej->config_check("keyboard.js");
    freej->interactive = false;
    freej->fps.set( fps );
    freej->start_running = startstate;
    addTextLayer();
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

bool Qfreej::getStartState()
{
    return(startstate);
}

void Qfreej::addLayer()
{
    QString fichier = QFileDialog::getOpenFileName(this, "Ouvrir un fichier", QString(), "All (*.*)");

    QqWidget *aWidget = new QqWidget(freej, tabWidget, this, fichier);
    if (!aWidget)
    {
        QMessageBox::information(this, "Layers", "Can't create TextLayer\n");
    }
}

void Qfreej::addTextLayer()
{
    QqWidget *aWidget = new QqWidget(freej, tabWidget, this);
    if (!aWidget)
    {
        QMessageBox::information(this, "Layers", "Can't create TextLayer\n");
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
