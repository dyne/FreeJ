
EMACS = emacs
INDEX = index.org

ifndef $(SCRIPTS)
	SCRIPTS = .
endif

all: cache ascii html pdf apache.conf

cache:
	mkdir -p templates_c cache
	chmod a+rwx templates_c cache

index.html: index.org
	$(EMACS) -no-site-file -batch -l $(SCRIPTS)/batch.el \
	 --visit $(INDEX) -f org-export-as-html

index.txt: index.org
	$(EMACS) -no-site-file  q -batch -l $(SCRIPTS)/batch.el \
	 --visit $(INDEX) -f org-export-as-ascii

index.pdf: index.org
	$(EMACS) -no-site-file -q -batch -l $(SCRIPTS)/batch.el \
	 --visit $(INDEX) -f org-export-as-pdf

ascii: index.txt

html: index.html
	 $(SCRIPTS)/org-split-body-toc.sh index.html

pdf: index.pdf

apache.conf:
	./generate-apache-conf.sh

clean:
	rm -f *~ *.pdf *.tex *.html *.xml *.txt *.conf
	rm -rf templates_c cache


