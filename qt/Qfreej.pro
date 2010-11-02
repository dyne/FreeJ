# -------------------------------------------------
# Project created by QtCreator 2010-10-04T17:27:39
# -------------------------------------------------
QT += network
 #   opengl \
 #   phonon \
 #   multimedia
TARGET = Qfreej
TEMPLATE = app
SOURCES += main.cpp \
    qfreej.cpp \
    QqComboBlit.cpp \
    QqWidget.cpp \
    QqComboFilter.cpp \
    QqFiltersApplied.cpp
    HEADERS += qfreej.h \
    QqComboBlit.h \
    QqWidget.h \
    QqComboFilter.h \
    QqFiltersApplied.h
FORMS += qfreej.ui
LIBS += -lfreej
INCLUDEPATH += ../src/include \
    ../ \
    /usr/include/SDL \
    ../lib/sdl_ttf
#    /home/fred/system/video/freej-git/src/freej-git-20100223/lib/javascript \
#    /home/fred/system/video/freej-git/src/freej-git-20100223/lib/flash
