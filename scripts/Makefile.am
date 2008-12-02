
DIRS    := $(shell for d in *; do test -d $$d && echo $$d; done)

install:
	for subdir in $(DIRS); do cp -ra $$subdir $(pkgdatadir); done

