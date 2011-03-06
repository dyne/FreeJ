AM_CFLAGS = $(QTGUI_CFLAGS) -DPREFIX="\"$(prefix)\"" -DDATADIR="\"$(datadir)\""
AM_CXXFLAGS = $(QTGUI_CFLAGS) -DPREFIX="\"$(prefix)\"" -DDATADIR="\"$(datadir)\""

SOURCES = main.cpp \
    qfreej.cpp \
    QqComboBlit.cpp \
    QqWidget.cpp \
    QqComboFilter.cpp \
    QqFiltersApplied.cpp \
    specialeventget.cpp \
    FakeWindow.cpp \
    QqTabWidget.cpp \
    Sound.cpp \
	qJackClient.cpp \
    qOpacity.cpp \
	qEncoder.cpp


bin_PROGRAMS = qfreej
qfreej_SOURCES = $(SOURCES)
qfreej_LDADD = $(QTGUI_LIBS)
