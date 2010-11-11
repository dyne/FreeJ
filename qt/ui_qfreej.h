/********************************************************************************
** Form generated from reading UI file 'qfreej.ui'
**
** Created: Wed Nov 10 11:54:58 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QFREEJ_H
#define UI_QFREEJ_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMdiArea>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Qfreej
{
public:
    QAction *actionOpen;
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QGridLayout *gridLayout_2;
    QGridLayout *gridLayout;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QMdiArea *mdiArea;
    QPushButton *streamButton;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *Qfreej)
    {
        if (Qfreej->objectName().isEmpty())
            Qfreej->setObjectName(QString::fromUtf8("Qfreej"));
        Qfreej->resize(562, 362);
        actionOpen = new QAction(Qfreej);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        centralWidget = new QWidget(Qfreej);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        horizontalLayout = new QHBoxLayout(centralWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setSpacing(6);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        widget = new QWidget(centralWidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        horizontalLayout_2 = new QHBoxLayout(widget);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        mdiArea = new QMdiArea(widget);
        mdiArea->setObjectName(QString::fromUtf8("mdiArea"));

        horizontalLayout_3->addWidget(mdiArea);


        horizontalLayout_2->addLayout(horizontalLayout_3);


        gridLayout->addWidget(widget, 1, 1, 1, 1);


        gridLayout_2->addLayout(gridLayout, 1, 0, 1, 1);

        streamButton = new QPushButton(centralWidget);
        streamButton->setObjectName(QString::fromUtf8("streamButton"));

        gridLayout_2->addWidget(streamButton, 0, 0, 1, 1);


        horizontalLayout->addLayout(gridLayout_2);

        Qfreej->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(Qfreej);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 562, 22));
        Qfreej->setMenuBar(menuBar);
        mainToolBar = new QToolBar(Qfreej);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        Qfreej->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(Qfreej);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        Qfreej->setStatusBar(statusBar);

        retranslateUi(Qfreej);

        QMetaObject::connectSlotsByName(Qfreej);
    } // setupUi

    void retranslateUi(QMainWindow *Qfreej)
    {
        Qfreej->setWindowTitle(QApplication::translate("Qfreej", "Qfreej", 0, QApplication::UnicodeUTF8));
        actionOpen->setText(QApplication::translate("Qfreej", "open", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionOpen->setToolTip(QApplication::translate("Qfreej", "Lance la lecture", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        streamButton->setToolTip(QApplication::translate("Qfreej", "Assuming you have a dummy filled Javasript streaming file named stream.js", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        streamButton->setText(QApplication::translate("Qfreej", "Stream", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Qfreej: public Ui_Qfreej {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QFREEJ_H
