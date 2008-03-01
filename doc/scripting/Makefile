# type make and this makefile will generate the html documentation

all:
	./jsdoc.pl -d html --project-name "<a href="http://freej.dyne.org">FreeJ</a> scripting" --logo "../ipernav-small.png" --page-footer "`cat footer.html`" \
	FreejScripting.js \
	Layer.js \
	GeometryLayer.js \
	Controller.js \
	VideoEncoder.js
	cp -a images html

clean:
	rm -rf html
