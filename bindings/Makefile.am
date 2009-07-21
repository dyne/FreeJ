if BUILD_SWIG_PYTHON
SWIG_PYTHON_DIR = python
endif

if BUILD_SWIG_RUBY
SWIG_RUBY_DIR = ruby
endif

if BUILD_SWIG_JAVA
SWIG_JAVA_DIR = java
endif

if BUILD_SWIG_PERL
SWIG_PERL_DIR = perl
endif

SUBDIRS = $(SWIG_PYTHON_DIR) \
          $(SWIG_RUBY_DIR) \
          $(SWIG_JAVA_DIR) \
          $(SWIG_PERL_DIR)

EXTRA_DIST = freej.i   \
             javapre.i \
             luapost.i \
             perlpre.i \
             pypost.i  \
             pypre.i   \
             rbpost.i  \
             rbpre.i   \
             test.lua
