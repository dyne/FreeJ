INCLUDES = -I$(top_srcdir)/lib/frei0r
plugindir = $(libdir)/freej

plugin_LTLIBRARIES = glow.la
glow_la_SOURCES = glow.c
glow_la_LDFLAGS = -module -avoid-version -share

# XXX(godog) this is an hack, frei0r needs updating
EXTRA_DIST = blur.c
