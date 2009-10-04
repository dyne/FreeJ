allsources = FreejScripting.js Layer.js GeometryLayer.js Controller.js \
	VideoEncoder.js AudioCollector.js GoomLayer.js footer.html

# where to build docs
htmlbuild = html
# final install dir
installdest = $(DESTDIR)$(docdir)/jsdoc

EXTRA_DIST = $(allsources) JSDoc JavaScript images templates .jsdoc_config \
			 JSDoc.pm jsdoc.pl stylesheet.css

noinst_DATA = jsdoc

jsdoc: $(htmlbuild)/index.html
$(htmlbuild)/index.html: $(allsources)
	$(INSTALL) -d $(htmlbuild)
	$(PERL) $(srcdir)/jsdoc.pl -d $(htmlbuild) \
		--project-name "<a href="@PACKAGE_URL@">@PACKAGE_NAME@</a> scripting" \
		--logo "$(srcdir)/../ipernav-small.png" \
		--page-footer "`cat $(srcdir)/footer.html`" \
		$(srcdir)
	cp -a $(srcdir)/images $(htmlbuild)


install-data-local:
	$(INSTALL) -d $(installdest)
	for d in `find $(htmlbuild) -type d`; do $(INSTALL) -m755 -d  $(installdest)/$$d; done
	for f in `find $(htmlbuild) -type f`; do $(INSTALL) -m644 $$f $(installdest)/$$f; done

uninstall-local:
	-rm -rf $(installdest)

clean-local:
	-rm -rf $(htmlbuild)
