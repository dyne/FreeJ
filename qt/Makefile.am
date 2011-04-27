AM_CXXFLAGS = -I$(top_srcdir)/src/include $(SDL_CFLAGS) \
	 $(QTGUI_CFLAGS) -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED \
	 -DPREFIX="\"$(prefix)\"" -DDATADIR="\"$(datadir)\""

ui_%.h: %.ui
	uic -o $@ $<
ui_%.cpp: %.ui
	uic -o $@ -impl $*.h $<
# This rule lets GNU make create any moc_*.cpp from the equivalent *.h
# You have one .h file, it's called myapp.h. Therefore, here I list
# its mocced name, moc_myapp.cpp.
moc_%.cpp: %.h
	moc $< -o $@

ui_qfreej.h:
	uic -o $(top_srcdir)/qt/ui_qfreej.h $(top_srcdir)/qt/ui_qfreej.ui

BUILT_SOURCES = ui_qfreej.h \
		moc_qfreej.cpp \
		moc_QqComboBlit.cpp \
		moc_QqWidget.cpp \
		moc_QqComboFilter.cpp \
		moc_QqFiltersApplied.cpp \
		moc_QqTabWidget.cpp \
		moc_qJackClient.cpp \
		moc_qOpacity.cpp \
		moc_qLogging.cpp \
		moc_qEncoder.cpp \
		moc_qqcombores.cpp


SOURCES = main.cpp \
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
	qqcombores.cpp \
    $(BUILT_SOURCES)

noinst_HEADERS = FakeWindow.h qJackClient.h QqComboBlit.h QqLinkList.h	\
		 specialeventget.h qEncoder.h qLogging.h		\
		 QqComboFilter.h QqTabWidget.h qfreej.h qOpacity.h	\
		 QqFiltersApplied.h QqWidget.h


bin_PROGRAMS = qfreej
qfreej_SOURCES = $(SOURCES)
qfreej_LDADD = $(top_builddir)/src/libfreej.la $(SDL_LIBS) $(QTGUI_LIBS) $(SHOUT_LIBS)

#clean:
#	rm -f qfreej *.o
#	rm -f moc_*.cpp moc_*.h
#	rm -f ui_*.h
