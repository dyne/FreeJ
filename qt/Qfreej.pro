# -------------------------------------------------
# Project created by QtCreator 2010-10-04T17:27:39
# -------------------------------------------------
#QT += network
 #   opengl \
 #   phonon \
 #   multimedia
TARGET = qfreej
TEMPLATE = app
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
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
CONFIG += qt no_lflags_merge
freej_release {
    CONFIG += release
    DEFINES += NDEBUG
} else {
    CONFIG += debug
}
FREEJ_PROFILE = $$system(pkg-config --variable=profile freej)
isEmpty(FREEJ_PROFILE): error(Set PKG_CONFIG_PATH to the installed FreeJ profile)
FREEJ_CAPABILITIES = $$system(pkg-config --variable=capabilities freej)
message(FreeJ $$FREEJ_PROFILE profile: $$FREEJ_CAPABILITIES)
QMAKE_CXXFLAGS += $$system(pkg-config --cflags freej)
LIBS += $$system(pkg-config --libs freej)
equals(FREEJ_PROFILE, static) {
    !contains(QT_CONFIG, static): error(The static FreeJ profile requires static Qt)
    QMAKE_LFLAGS += -static
}

target.path = $$system(pkg-config --variable=prefix freej)/bin
INSTALLS += target
