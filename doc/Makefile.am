
# TODO(godog): build only if perl and perl modules required by jsdoc have been
# found (html::template)
SUBDIRS = scripting

EXTRA_DIST = doxy2swig.py freej.1 ipernav.png ipernav-small.png

man_MANS = freej.1

# TODO(shammash):
# - look for doxygen autotools support
# - check for doxygen and python while configuring
# - make install
pydocstrings.i:
	doxygen freej.dox
	python $(srcdir)/doxy2swig.py xml/index.xml $@

noinst_DATA = pydocstrings.i

clean-local:
	rm -rf html
	rm -rf xml
	rm -f pydocstrings.i

