allsources = FreejScripting.js Layer.js GeometryLayer.js Controller.js \
	VideoEncoder.js AudioCollector.js GoomLayer.js footer.html

# where to build docs
htmlbuild = html
# final install dir
installdest = $(DESTDIR)$(docdir)/scripting

EXTRA_DIST = $(allsources) JSDoc JavaScript images templates .jsdoc_config \
			 JSDoc.pm jsdoc.pl stylesheet.css

noinst_DATA = jsdoc

# TODO(godog): parametrize $perl and invoke as $perl ./jsdoc.pl
jsdoc: $(htmlbuild)/index.html
$(htmlbuild)/index.html: $(allsources)
	$(srcdir)/jsdoc.pl -d $(htmlbuild) \
		--project-name "<a href="http://freej.dyne.org">FreeJ</a> scripting" \
		--logo "$(srcdir)/../ipernav-small.png" \
		--page-footer "`cat $(srcdir)/footer.html`" \
		$(srcdir)
	cp -a $(srcdir)/images $(htmlbuild)

install-data-hook:
	$(MKDIR_P) $(installdest)
	cp -a $(htmlbuild) $(installdest)

uninstall-local:
	-rm -rf $(installdest)

clean-local:
	-rm -rf $(htmlbuild)
