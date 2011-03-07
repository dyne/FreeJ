AM_CXXFLAGS = -I$(top_srcdir)/src/include $(SDL_CFLAGS) \
	 $(QTGUI_CFLAGS) -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED \
	 -DPREFIX="\"$(prefix)\"" -DDATADIR="\"$(datadir)\""

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
	qEncoder.cpp \
 moc_qfreej.cpp \
		moc_QqComboBlit.cpp \
		moc_QqWidget.cpp \
		moc_QqComboFilter.cpp \
		moc_QqFiltersApplied.cpp \
		moc_QqTabWidget.cpp \
		moc_Sound.cpp \
		moc_qJackClient.cpp \
		moc_qOpacity.cpp \
		moc_qEncoder.cpp


bin_PROGRAMS = qfreej
qfreej_SOURCES = $(SOURCES)
qfreej_LDADD = -lfreej $(SDL_LIBS) $(QTGUI_LIBS)
