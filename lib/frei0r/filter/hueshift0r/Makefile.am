INCLUDES = -I$(top_srcdir)/lib/frei0r
plugindir = $(libdir)/freej

plugin_LTLIBRARIES = hueshift0r.la
hueshift0r_la_SOURCES = hueshift0r.c
hueshift0r_la_LDFLAGS = -module -avoid-version -shared

# XXX(godog) this is an hack, frei0r needs updating
noinst_HEADERS = matrix.h
