if BUILD_JSAPIDOC
SCRIPTINGDIR = scripting
endif
SUBDIRS = $(SCRIPTINGDIR)

EXTRA_DIST = doxy2swig.py freej.1 ipernav.png ipernav-small.png

man_MANS = freej.1


clean-local:
	rm -rf html
	rm -rf xml
	rm -f pydocstrings.i

