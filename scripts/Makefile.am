
scriptsdir = "$(DESTDIR)$(prefix)/share/doc/$(PACKAGE)/scripts"

DIRS    = `for x in $(top_srcdir)/$(subdir)/*; do test -d $$x && echo $$x; done`

install:
	test -z "$(scriptsdir)" || mkdir -p "$(scriptsdir)"
	for subdir in $(DIRS); do cp -ra $$subdir "$(scriptsdir)"; done

uninstall:
	for subdir in $(DIRS); do rm -rf "$(scriptsdir)/$$subdir" ; done
	rm -rf "$(scriptsdir)"

# TODO(shammash): 'make dist' does not work here!!!

