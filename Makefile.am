dist_docs_DATA = README README.OSX TODO INSTALL API COPYING USAGE AUTHORS NEWS KNOWN-BUGS
docsdir = ${prefix}/share/doc/${PACKAGE}

DISTCHECK_CONFIGURE_FLAGS = --enable-ruby --enable-python

#pkgdatadir = ${prefix}/share/freej

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = freej.pc

SUBDIRS = lib src doc bindings scripts tests $(EXTRA_DIRS)

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = autogen.sh freej.spec xcode
