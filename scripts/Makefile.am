
scriptsdir = "$(DESTDIR)$(prefix)/share/doc/$(PACKAGE)/scripts"

DIRS    := $(shell for d in *; do test -d $$d && echo $$d; done)

install:
	test -z "$(scriptsdir)" || mkdir -p "$(scriptsdir)"
	for subdir in $(DIRS); do cp -ra $$subdir "$(scriptsdir)"; done

uninstall:
	for subdir in $(DIRS); do rm -rf "$(scriptsdir)/$$subdir" ; done
	rm -rf "$(scriptsdir)"

