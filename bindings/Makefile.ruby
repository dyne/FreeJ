
SHELL = /bin/sh

#### Start of system configuration section. ####

srcdir = .
topdir = /usr/include/ruby-1.9
hdrdir = /usr/include/ruby-1.9
arch_hdrdir = /usr/include/ruby-1.9/$(arch)
VPATH = $(srcdir):$(arch_hdrdir)/ruby:$(hdrdir)/ruby
prefix = $(DESTDIR)/usr
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
sbindir = $(exec_prefix)/sbin
libexecdir = $(prefix)/lib/ruby1.9
datarootdir = $(prefix)/share
datadir = $(datarootdir)
sysconfdir = $(DESTDIR)/etc
sharedstatedir = $(prefix)/com
localstatedir = $(DESTDIR)/var
includedir = $(prefix)/include
oldincludedir = $(DESTDIR)/usr/include
docdir = $(datarootdir)/doc/$(PACKAGE)
infodir = $(prefix)/share/info
htmldir = $(docdir)
dvidir = $(docdir)
pdfdir = $(docdir)
psdir = $(docdir)
libdir = $(exec_prefix)/lib
localedir = $(datarootdir)/locale
mandir = $(prefix)/share/man
sitedir = $(DESTDIR)/usr/local/lib/site_ruby
rubyhdrdir = $(includedir)/ruby-$(MAJOR).$(MINOR)
sitehdrdir = $(rubyhdrdir)/site_ruby
rubylibdir = $(libdir)/ruby/$(ruby_version)
archdir = $(rubylibdir)/$(arch)
sitelibdir = $(sitedir)/$(ruby_version)
sitearchdir = $(sitelibdir)/$(sitearch)

CC = cc
CXX = @g++
LIBRUBY = $(LIBRUBY_SO)
LIBRUBY_A = lib$(RUBY_SO_NAME)-static.a
LIBRUBYARG_SHARED = -l$(RUBY_SO_NAME)
LIBRUBYARG_STATIC = -l$(RUBY_SO_NAME)-static

RUBY_EXTCONF_H = 
CFLAGS   =  -fPIC -fno-strict-aliasing -g -c -fPIC -I../src/include -I/usr/include/SDL -DXP_UNIX -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/freetype2 -DPREFIX="\"/usr/local\"" -DDATADIR="\"/usr/local/share\"" -I../lib/shout -I../ -I../lib/javascript
INCFLAGS = -I. -I$(arch_hdrdir) -I$(hdrdir) -I$(srcdir)
CPPFLAGS =  
CXXFLAGS = $(CFLAGS) -fno-strict-aliasing -g
DLDFLAGS = -L.  -rdynamic -Wl,-export-dynamic ../src/libfreej.a -I../src/include -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/freetype2 -DPREFIX="\"/usr/local\"" -DDATADIR="\"/usr/local/share\"" -D_REENTRANT -Wall -I../ -I../lib/javascript -I../lib/shout ../lib/ccvt/libccvt.a ../lib/sdl_image/libsdl_image.a ../lib/sdl_gfx/libsdl_gfx.a ../lib/sdl_ttf/libsdl_ttf.a ../lib/javascript/obj/libjs.a ../lib/flash/libflash.a ../lib/goom/libgoom2.a ../lib/shout/libshout.a ../lib/lo/liblo.a ../lib/cwiid/libwiimote.a -ldl -lslang -lpng /usr/lib/libjpeg.so -L/usr/lib /usr/lib/libSDL.so /usr/lib/libfreetype.so -lz /usr/lib/libasound.so /usr/lib/libjack.so -lpthread -lrt /usr/lib/libfftw3.so -lavformat -lavcodec /usr/lib/libtheora.so /usr/lib/libvorbisenc.so /usr/lib/libraw1394.so /usr/lib/libvorbis.so -lm /usr/lib/libogg.so -lavutil -lX11 /usr/lib/libbluetooth.so -lGL -lGLU -shared -o Freej.so -DXP_UNIX -I//usr/include/ruby-1.9/i486-linux -I//usr/include/ruby-1.9/
LDSHARED = $(CC) -shared
LDSHAREDXX = $(CXX) -shared
AR = ar
EXEEXT = 

RUBY_INSTALL_NAME = ruby1.9
RUBY_SO_NAME = ruby1.9
arch = i486-linux
sitearch = i486-linux
ruby_version = 1.9
ruby = /usr/bin/ruby1.9
RUBY = $(ruby)
RM = rm -f
MAKEDIRS = mkdir -p
INSTALL = /usr/bin/install -c
INSTALL_PROG = $(INSTALL) -m 0755
INSTALL_DATA = $(INSTALL) -m 644
COPY = cp

#### End of system configuration section. ####

preload = 

libpath = . $(libdir)
LIBPATH =  -L. -L$(libdir)
DEFFILE = 

CLEANFILES = mkmf.log
DISTCLEANFILES = 

extout = 
extout_prefix = 
target_prefix = 
LOCAL_LIBS = 
LIBS = $(LIBRUBYARG_SHARED) -lsupc++  -lpthread -ldl -lcrypt -lm   -lc
SRCS = freej_rby.cpp
OBJS = freej_rby.o
TARGET = example
DLLIB = $(TARGET).so
EXTSTATIC = 
STATIC_LIB = 

RUBYCOMMONDIR = $(sitedir)$(target_prefix)
RUBYLIBDIR    = $(sitelibdir)$(target_prefix)
RUBYARCHDIR   = $(sitearchdir)$(target_prefix)
HDRDIR        = $(rubyhdrdir)/ruby$(target_prefix)
ARCHHDRDIR    = $(rubyhdrdir)/$(arch)/ruby$(target_prefix)

TARGET_SO     = $(DLLIB)
CLEANLIBS     = $(TARGET).so $(TARGET).il? $(TARGET).tds $(TARGET).map
CLEANOBJS     = *.o *.a *.s[ol] *.pdb *.exp *.bak

all:		$(DLLIB)
static:		$(STATIC_LIB)

clean:
		@-$(RM) $(CLEANLIBS) $(CLEANOBJS) $(CLEANFILES)

distclean:	clean
		@-$(RM) Makefile $(RUBY_EXTCONF_H) conftest.* mkmf.log
		@-$(RM) core ruby$(EXEEXT) *~ $(DISTCLEANFILES)

realclean:	distclean
install: install-so install-rb

install-so: $(RUBYARCHDIR)
install-so: $(RUBYARCHDIR)/$(DLLIB)
$(RUBYARCHDIR)/$(DLLIB): $(DLLIB)
	$(INSTALL_PROG) $(DLLIB) $(RUBYARCHDIR)
install-rb: pre-install-rb install-rb-default
install-rb-default: pre-install-rb-default
pre-install-rb: Makefile
pre-install-rb-default: Makefile
$(RUBYARCHDIR):
	$(MAKEDIRS) $@

site-install: site-install-so site-install-rb
site-install-so: install-so
site-install-rb: install-rb

.SUFFIXES: .c .m .cc .cxx .cpp .C .o

.cc.o:
	$(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $<

.cxx.o:
	$(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $<

.cpp.o:
	$(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $<

.C.o:
	$(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $<

.c.o:
	$(CC) $(INCFLAGS) $(CPPFLAGS) $(CFLAGS) -c $<

$(DLLIB): $(OBJS)
	@-$(RM) $(@)
	$(LDSHAREDXX) -o $@ $(OBJS) $(LIBPATH) $(DLDFLAGS) $(LOCAL_LIBS) $(LIBS)



$(OBJS): ruby.h defines.h
