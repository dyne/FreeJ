# -------------------------------------------------
# Project created by QtCreator 2010-10-04T17:27:39
# -------------------------------------------------
#QT += network
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
    QqFiltersApplied.cpp \
    specialeventget.cpp \
    FakeWindow.cpp \
    QqTabWidget.cpp \
    Sound.cpp

    HEADERS += qfreej.h \
    QqComboBlit.h \
    QqWidget.h \
    QqComboFilter.h \
    QqFiltersApplied.h \
    specialeventget.h \
    FakeWindow.h \
    QqTabWidget.h \
    Sound.h
FORMS += qfreej.ui
CFLAGS += g
LIBS += -lfreej
INCLUDEPATH += ../ \
    /usr/include/SDL \
    ../lib/sdl_ttf \
    ../src/include
