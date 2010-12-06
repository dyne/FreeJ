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
#include <generator_layer.h>

using namespace std;
extern QSize viewSize;

Qfreej::Qfreej(QWidget *parent) :
        QWidget(parent)
{
    m_snd = NULL;

    grid = new QGridLayout(this);

    QMenuBar *menuBar = new QMenuBar(this);
    grid->addWidget(menuBar, 0, 0);
    QMenu *menuFichier = menuBar->addMenu("&Fichier");

    QAction *actionPen = menuFichier->addAction("&Open");
    connect(actionPen, SIGNAL(triggered()), this, SLOT(addLayer()));

    QAction *actionAddTextLayer = menuFichier->addAction("&Text layer");
    connect(actionAddTextLayer, SIGNAL(triggered()), this, SLOT(addTextLayer()));

    QAction *actionStream = menuFichier->addAction("&Run js");
    connect(actionStream, SIGNAL(triggered()), this, SLOT(startStreaming()));//new

    QAction *actionJack = menuFichier->addAction("&Stream");
    connect(actionJack, SIGNAL(triggered()), this, SLOT(openSoundDevice()));//new

    menuGenerator = menuFichier->addMenu("&Generators");

    poller = new QTimer (this);
    connect(poller, SIGNAL(timeout()), this, SLOT(updateInterface()));
    poller->start(10); //start timer to trigger every 10 ms the updateInterface slot

    freej = new Context();
    init();
    number = 0;
    setAttribute(Qt::WA_DeleteOnClose);
}

Qfreej::~Qfreej()
{
    if (m_snd)
        delete m_snd;
    poller->stop();
//    delete freej;     //to be investigate :)
//    delete layoutH;
    delete grid;
}

void Qfreej::closeEvent(QCloseEvent *event)
{
    event->accept();
    delete freej;
}

void Qfreej::init()
{
    tabWidget = new QqTabWidget(this);
    grid->addWidget(tabWidget, 1, 0);
    setLayout(grid);//new
    tabWidget->setMovable(true);
    tabWidget->setTabsClosable(true);

    myTabBar = tabWidget->getTabBar();
    myTabBar->setTabsClosable(true);
    myTabBar->setAttribute(Qt::WA_DeleteOnClose);
    connect(myTabBar, SIGNAL(tabMoved(int, int)), tabWidget, SLOT(moveLayer(int, int)));

    startstate = true;
//    fps = 25;                   //fps par défaut
    screen = Factory<ViewPort>::get_instance( "Screen", "sdl" );
    screen->init(viewSize.width(), viewSize.height(), 32);
    freej->add_screen(screen);
    freej->config_check("keyboard.js");
    freej->interactive = false;
//    freej->fps.set( fps );
    freej->start_running = startstate;

    createMenuGenerator();
}

void Qfreej::startStreaming()
{
    QString fichier = QFileDialog::getOpenFileName(this, "Ouvrir un fichier JavaScript", QString(), "All (*.js)");
    if (fichier != "")
    {
        freej->open_script((char *)(fichier.toStdString().c_str()));
    }
}

bool Qfreej::getStartState()
{
    return(startstate);
}

void Qfreej::addLayer()
{
    QString fichier = QFileDialog::getOpenFileName(this, "Ouvrir un fichier", QString(), "All (*.*)");

    if (fichier != "")
    {
        QqWidget *aWidget = new QqWidget(freej, tabWidget, this, fichier);
        if (!aWidget)
        {
            QMessageBox::information(this, "Layers", "Can't create TextLayer\n");
        }
    }
}

void Qfreej::openSoundDevice()
{
    m_snd = new Sound(freej);
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
    if (!freej->quit)
        freej->cafudda(1.0);
}


Context *Qfreej::getContext()
{
    return (freej);
}

//fills the generator menu
void Qfreej::createMenuGenerator()
{
    Filter **res;
    Filter *filt;
    int c;

    res = freej->generators.completion("");
    if(!res[0]) {
      qDebug() << "no generators found";
      return;
    }
    for(c=0;res[c];c++) {
        filt = res[c];
        if(!filt) break;
        menuGenerator->addAction(filt->name);
    }
    connect(menuGenerator, SIGNAL(triggered(QAction*)), this, SLOT(addGenerator(QAction*)));
}

void Qfreej::addGenerator(QAction* action)
{
    QqWidget *aWidget = new QqWidget(freej, tabWidget, this, action);
    if (!aWidget)
    {
        QMessageBox::information(this, "GeneratorLayer", "Can't create the " + action->text() + "generator layer");
    }
}
