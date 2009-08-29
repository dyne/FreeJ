AM_CPPFLAGS = -I$(top_srcdir)/src/include $(SLANG_CFLAGS)
AM_LDFLAGS = $(SLANG_LIBS)

AUTOMAKE_OPTIONS = foreign

noinst_LTLIBRARIES = libslw.la

libslw_la_SOURCES = slw_console.cpp slw.cpp \
                    slw_popup.cpp abs_text_console.cpp slw_text.cpp \
                    slw_log.cpp slw_prompt.cpp

noinst_HEADERS = abs_text_console.h keycodes.h slw_console.h slw.h \
	slw_log.h slw_popup.h slw_prompt.h slw_text.h

#	$(top_srcdir)/linklist.o $(top_srcdir)/jutils.o


