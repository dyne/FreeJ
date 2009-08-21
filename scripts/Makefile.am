scriptsdir = "$(DESTDIR)$(docdir)/scripts"

DIRS = javascript python ruby

install-data-local:
	$(INSTALL) -d $(scriptsdir)
	( cd $(srcdir); \
	for subdir in $(DIRS); do \
		for d in `find $$subdir -type d`; do $(INSTALL) -m755 -d  $(scriptsdir)/$$d; done ;\
		for f in `find $$subdir -type f`; do $(INSTALL) -m644 $$f $(scriptsdir)/$$f; done ;\
	done )

uninstall-local:
	for subdir in $(DIRS); do rm -rf "$(scriptsdir)/$$subdir"; done
	-rm -rf "$(scriptsdir)"

EXTRA_DIST = $(DIRS)
