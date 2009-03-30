
all:
	doxygen freej.dox
	make -C scripting
	python doxy2swig.py xml/index.xml pydocstrings.i


clean:
	rm -rf html
	rm -rf xml
	rm -f pydocstrings.i

