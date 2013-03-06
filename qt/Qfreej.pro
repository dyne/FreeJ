# -------------------------------------------------
# Project created by QtCreator 2010-10-04T17:27:39
# -------------------------------------------------
#QT += network
 #   opengl \
 #   phonon \
 #   multimedia
TARGET = qfreej
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
	qJackClient.cpp \
    qOpacity.cpp \
	qEncoder.cpp \
	qLogging.cpp \
    qqcombores.cpp

    HEADERS += qfreej.h \
    QqComboBlit.h \
    QqWidget.h \
    QqComboFilter.h \
    QqFiltersApplied.h \
    specialeventget.h \
    FakeWindow.h \
    QqTabWidget.h \
	qJackClient.h \
    qOpacity.h \
	qEncoder.h \
	qLogging.h \
    qqcombores.h
#FORMS += qfreej.ui
CONFIG += qt debug
LIBS += ../src/.libs/libfreej.a ../lib/javascript/libmozjs.a \
 `pkg-config --libs sdl cairo cwiid libavcodec libavformat libavutil libswscale \
                    shout jack fftw3 samplerate liblo` -lSDL_gfx -lSDL_ttf -lSDL_image
INCLUDEPATH += ../ \
    /usr/include/SDL \
    ../lib/sdl_ttf \
    ../src/include
