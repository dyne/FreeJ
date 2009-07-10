INCLUDES = -I$(top_srcdir)/lib/frei0r
plugindir = $(libdir)/freej

plugin_LTLIBRARIES = mask0mate.la
mask0mate_la_SOURCES = mask0mate.c
mask0mate_la_LDFLAGS = -module -avoid-version -shared

# XXX(godog) this is an hack, frei0r needs updating
EXTRA_DIST = blur.c
