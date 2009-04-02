
# TODO(shammash): include it later because it's not directory independent
#SUBDIRS = scripting

man_MANS = freej.1

# TODO(shammash): this is far from being autotoolized
# (at least we must check for doxygen and python)
pydocstrings.i:
	doxygen freej.dox
	python $(srcdir)/doxy2swig.py xml/index.xml $@

noinst_DATA = pydocstrings.i

clean-local:
	rm -rf html
	rm -rf xml
	rm -f pydocstrings.i

# TODO(shammash): dist and install ??
