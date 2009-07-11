AC_DEFUN([BIND_JAVA_SCRIPTING],
[

java_enable=no
java=no

AC_MSG_CHECKING([for Java])

AC_ARG_ENABLE(java,
	[  --enable-java             enable Java language bindings (no)],
	[java_enable=yes],[java_enable=no])

AC_MSG_RESULT($java_enable)

if test "$java_enable" = "yes"; then
   have_java=true
   AC_SUBST(have_java)
   JAVA_CFLAGS="-W -Wall -static-libgcc"
   JAVA_LDFLAGS="-norunpath -xnolib"
   AC_SUBST(JAVA_CFLAGS)
   AC_SUBST(JAVA_LDFLAGS)
fi

])

dnl i'll give m4 a try for perl... :S -jrml
AC_DEFUN([PERL_SWIG_BINDINGS],
[

AC_MSG_CHECKING([for Perl])

AC_ARG_ENABLE(perl,
 	[  --enable-perl             enable Perl support (no)],
	[enable_perl=yes],
	[enable_perl=no])

if test "$enable_perl" != "no"; then
   enable_perl=yes
   have_perl=true
   AC_SUBST(have_perl)
fi

AC_MSG_RESULT($enable_perl)

PERL_PATH=$PATH
AC_PATH_PROG(enable_perl, perl, no, $PERL_PATH)

PERL_ARCHLIB="`perl -e 'use Config; print $Config{archlib};'`"
AC_SUBST(PERL_ARCHLIB)

PERL_CFLAGS="`perl -e 'use Config; print $Config{ccflags};'`"
AC_SUBST(PERL_CFLAGS)




])

dnl Thank you very much Vim for this lovely ruby configuration
dnl The hitchhiked code is from Vim configure.in version 1.98


AC_DEFUN([EL_CONFIG_SCRIPTING_RUBY],
[

AC_DEFINE(SIGNED_VALUE,long,[signed value used by ruby])
AC_CACHE_CHECK(whether right shift preserve sign bit, rb_cv_rshift_sign,
    [AC_TRY_RUN([
int
main()
{
  if (-1==(-1>>1))
    return 0;
  return 1;
}
],
	rb_cv_rshift_sign=yes,
	rb_cv_rshift_sign=no,
	rb_cv_rshift_sign=yes)])
# if test "$rb_cv_rshift_sign" = yes; then
#   AC_DEFINE(RSHIFT(x,y), ((x)>>(int)y), [whether right shift preserve sign bit, rb_cv_rshift_sign])
# else
#   AC_DEFINE(RSHIFT(x,y), (((x)<0) ? ~((~(x))>>y) : (x)>>y), [whether right shift preserve sign bit, rb_cv_rshift_sign])
# fi


AC_MSG_CHECKING([for Ruby])

CONFIG_SCRIPTING_RUBY_WITHVAL="no"
CONFIG_SCRIPTING_RUBY="no"

dnl EL_SAVE_FLAGS

# AC_ARG_WITH(ruby,
# 	[  --with-ruby             enable Ruby support],
# 	[CONFIG_SCRIPTING_RUBY_WITHVAL="$withval"])

AC_ARG_ENABLE(ruby,
 	[  --enable-ruby             enable Ruby support (no)],
	[CONFIG_SCRIPTING_RUBY_WITHVAL=yes],
	[CONFIG_SCRIPTING_RUBY_WITHVAL=no])

if test "$CONFIG_SCRIPTING_RUBY_WITHVAL" != no; then
	CONFIG_SCRIPTING_RUBY="yes"
fi

AC_MSG_RESULT($CONFIG_SCRIPTING_RUBY)

if test "$CONFIG_SCRIPTING_RUBY" = "yes"; then
	if test -d "$CONFIG_SCRIPTING_RUBY_WITHVAL"; then
		RUBY_PATH="$CONFIG_SCRIPTING_RUBY_WITHVAL:$PATH"
	else
		RUBY_PATH="$PATH"
	fi

	AC_PATH_PROG(CONFIG_SCRIPTING_RUBY, ruby1.8, no, $RUBY_PATH)
	if test "$CONFIG_SCRIPTING_RUBY" != "no"; then

# 		AC_MSG_CHECKING(Ruby version)
# 		if $CONFIG_SCRIPTING_RUBY -e 'exit((VERSION or RUBY_VERSION) >= "1.6.0")' >/dev/null 2>/dev/null; then
			ruby_version=`$CONFIG_SCRIPTING_RUBY -e 'puts "#{VERSION rescue RUBY_VERSION}"'`
			AC_MSG_RESULT($ruby_version)

			dnl   -- Is this the right rationale?
			dnl   if we install in /usr/local then provide /lib/site_ruby/...
			dnl   otherwise provide /lib/ruby/...
			if test "X$prefix" = "X/usr/local" || test "X$prefix" = "XNONE" ; then
				rubyinstdir=`$CONFIG_SCRIPTING_RUBY -r rbconfig -e 'print Config::CONFIG[["sitelibdir"]].sub("/usr/local", "")' 2>/dev/null`
				rubyarchinstdir=`$CONFIG_SCRIPTING_RUBY -r rbconfig -e 'print Config::CONFIG[["sitearchdir"]].sub("/usr/local", "")' 2>/dev/null`
			else
				rubyinstdir=`$CONFIG_SCRIPTING_RUBY -r rbconfig -e 'print Config::CONFIG[["rubylibdir"]].sub(Config::CONFIG[["exec_prefix"]], "")' 2>/dev/null`
				rubyarchinstdir=`$CONFIG_SCRIPTING_RUBY -r rbconfig -e 'print Config::CONFIG[["archdir"]].sub(Config::CONFIG[["exec_prefix"]], "")' 2>/dev/null`
			fi
			AC_MSG_CHECKING(for Ruby header files)
			rubyhdrdir=`$CONFIG_SCRIPTING_RUBY -r mkmf -e 'print Config::CONFIG[["rubyhdrdir"]] || $hdrdir' 2>/dev/null`
			rubyarchdir=`$CONFIG_SCRIPTING_RUBY -r rbconfig -e 'print Config::CONFIG[["archdir"]]' 2>/dev/null`
			if test "X$rubyhdrdir" != "X"; then
				AC_MSG_RESULT($rubyhdrdir)
				RUBY_CFLAGS="-I$rubyhdrdir -I$rubyhdrdir/ruby -I$rubyhdrdir/`basename $rubyarchdir` -DRUBY_MISSING_H"
				rubylibs=`$CONFIG_SCRIPTING_RUBY -r rbconfig -e 'print Config::CONFIG[["LIBS"]]'`

				if test "X$rubylibs" != "X"; then
					RUBY_LIBS="$rubylibs"
				fi

				librubyarg=`$CONFIG_SCRIPTING_RUBY -r rbconfig -e 'print Config.expand(Config::CONFIG[["LIBRUBYARG"]])'`

				if test -f "$rubyhdrdir/$librubyarg"; then
					librubyarg="$rubyhdrdir/$librubyarg"

				else
					rubylibdir=`$CONFIG_SCRIPTING_RUBY -r rbconfig -e 'print Config.expand(Config::CONFIG[["libdir"]])'`
					if test -f "$rubylibdir/$librubyarg"; then
						librubyarg="$rubylibdir/$librubyarg"
					elif test "$librubyarg" = "libruby.a"; then
						dnl required on Mac OS 10.3 where libruby.a doesn't exist
						librubyarg="-lruby"
					else
						librubyarg=`$CONFIG_SCRIPTING_RUBY -r rbconfig -e "print '$librubyarg'.gsub(/-L\./, %'-L#{Config.expand(Config::CONFIG[\"libdir\"])}')"`
					fi
				fi

				if test "X$librubyarg" != "X"; then
					RUBY_LIBS="$librubyarg $RUBY_LIBS"
				fi

				rubyldflags=`$CONFIG_SCRIPTING_RUBY -r rbconfig -e 'print Config::CONFIG[["LDFLAGS"]]'`
				if test "X$rubyldflags" != "X"; then
					RUBY_LDFLAGS="$rubyldflags $RUBY_LDFLAGS"
				fi

				AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <ruby.h>]], [[ruby_init();]])],[CONFIG_SCRIPTING_RUBY=yes],[CONFIG_SCRIPTING_RUBY=no])
			else
				AC_MSG_RESULT([Ruby header files not found])
			fi
# 		else
# 			AC_MSG_RESULT(too old; need Ruby version 1.6.0 or later)
# 		fi
	fi

	have_ruby=true
	AC_SUBST(have_ruby)
	RUBY_INSTDIR=$rubyinstdir
	RUBY_ARCHDIR=$rubyarchdir
	RUBY_ARCHINSTDIR=$rubyarchinstdir
	AC_SUBST(RUBY_INSTDIR)
	AC_SUBST(RUBY_ARCHDIR)
	AC_SUBST(RUBY_ARCHINSTDIR)
#	LIBS="$LIBS $RUBY_LIBS"
	AC_SUBST(RUBY_CFLAGS)
	AC_SUBST(RUBY_LDFLAGS)
	AC_SUBST(RUBY_LIBS)

fi

dnl EL_RESTORE_FLAGS

# if test "$CONFIG_SCRIPTING_RUBY" != "yes"; then
# 	if test -n "$CONFIG_SCRIPTING_RUBY_WITHVAL" &&
# 	   test "$CONFIG_SCRIPTING_RUBY_WITHVAL" != no; then
# 		AC_MSG_ERROR([Ruby not found])
# 	fi
# else
dnl	EL_CONFIG(CONFIG_SCRIPTING_RUBY, [Ruby])
# fi
])
