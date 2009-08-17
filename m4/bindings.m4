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


dnl ################# new macros ##################################

dnl how directories are affected wrt INSTALLDIRS (ExtUtils::MakeMaker)
dnl                        perl            site                vendor
dnl                        PERLPREFIX      SITEPREFIX          VENDORPREFIX
dnl         INST_ARCHLIB   INSTALLARCHLIB  INSTALLSITEARCH     INSTALLVENDORARCH
dnl         INST_LIB       INSTALLPRIVLIB  INSTALLSITELIB      INSTALLVENDORLIB
dnl         INST_BIN       INSTALLBIN      INSTALLSITEBIN      INSTALLVENDORBIN
dnl         INST_SCRIPT    INSTALLSCRIPT   INSTALLSITESCRIPT   INSTALLVENDORSCRIPT
dnl         INST_MAN1DIR   INSTALLMAN1DIR  INSTALLSITEMAN1DIR  INSTALLVENDORMAN1DIR
dnl         INST_MAN3DIR   INSTALLMAN3DIR  INSTALLSITEMAN3DIR  INSTALLVENDORMAN3DIR

dnl TODO
dnl - support INSTALLDIRS "perl" style?

dnl ac_perl_dirs variable_to_set dir_type (arch, lib, bin, script, man1dir, man3dir)
AC_DEFUN([AC_PERL_INSTALLDIRS],
[
    AC_ARG_WITH([perl-installdirs],
        AS_HELP_STRING([--with-perl-installdirs],[style of perl install
            directories, vendor/site (site)]),
        [perl_dirs=$withval],
        [perl_dirs=site])

    AC_MSG_CHECKING([for perl $2 dirs])

    # use a temp variable to avoid needless quoting inside backticks together with expansion
    ac_perl_dirs_prog="use Config; print \$Config{install${perl_dirs}$2}"
    $1="`$PERL -e \"$ac_perl_dirs_prog\"`"
    AC_MSG_RESULT($$1)
])

AC_DEFUN([SWIG_PERL],
[
    if test -z "$PERL"; then
            AC_PATH_PROG([PERL], [perl])
    fi

    if test -z "$PERL"; then
            AC_MSG_ERROR([Can't find perl interpreter in PATH, set PERL])
    fi

    AC_PERL_INSTALLDIRS([PERL_INSTALLARCH], arch)
    AC_SUBST([PERL_INSTALLARCH])

    AC_PERL_INSTALLDIRS([PERL_INSTALLLIB], lib)
    AC_SUBST([PERL_INSTALLLIB])

    AX_PERL_EXT_CFLAGS([PERL_CFLAGS])
    AC_SUBST([PERL_CFLAGS])

    AX_PERL_EXT_LDFLAGS([PERL_LDFLAGS])
    AC_SUBST([PERL_LDFLAGS])
])

AC_DEFUN([ENABLE_SWIG_PERL],
[
    AC_ARG_ENABLE(perl,
    AS_HELP_STRING([--enable-perl],[enable Perl bindings (no)]),
    [enable_perl=$enableval],
    [enable_perl=no])

if test x"$enable_perl" = xyes; then
	SWIG_PERL
fi
])

AC_DEFUN([ENABLE_SWIG_PYTHON],
[
AC_ARG_ENABLE(python,
    AS_HELP_STRING([--enable-python],[enable Python bindings (no)]),
	[enable_python=$enableval],
	[enable_python=no])

if test x"$enable_python" = xyes; then
	AC_PYTHON_DEVEL
	AM_PATH_PYTHON(2.4)
	SWIG_PYTHON
fi
])

AC_DEFUN([ENABLE_SWIG_JAVA],
[
AC_ARG_ENABLE(java,
    AS_HELP_STRING([--enable-java],[enable Java bindings (no)]),
	[enable_java=$enableval],
	[enable_java=no])

if test x"$enable_java" = x"yes"; then
   JAVA_CFLAGS="-W -Wall -static-libgcc"
   JAVA_LDFLAGS="-norunpath -xnolib"
   AC_SUBST(JAVA_CFLAGS)
   AC_SUBST(JAVA_LDFLAGS)
fi
])

AC_DEFUN([ENABLE_SWIG_RUBY],
[
AC_ARG_ENABLE(ruby,
    AS_HELP_STRING([--enable-ruby],[enable Ruby bindings (no)]),
                   [enable_ruby=$enableval],
                   [enable_ruby=no])

if test x"$enable_ruby" = x"yes"; then
    AX_WITH_RUBY
    if test -z "$RUBY"; then
       AC_MSG_ERROR([Can't find ruby interpreter in PATH])
    fi

    AX_RUBY_DEVEL

    dnl   -- Is this the right rationale?
    dnl   if we install in /usr/local then provide /lib/site_ruby/...
    dnl   otherwise provide /lib/ruby/...
    if test "X$prefix" = "X/usr/local" || test "X$prefix" = "XNONE" ; then
        RUBY_SITE_LIB=`$RUBY -rrbconfig -e'print Config::CONFIG[["sitelibdir"]].sub("/usr/local", "")' 2>/dev/null`
        RUBY_SITE_PKG=`$RUBY -rrbconfig -e'print Config::CONFIG[["sitearchdir"]].sub("/usr/local", "")' 2>/dev/null`
    else
        RUBY_SITE_LIB=`$RUBY -rrbconfig -e'print Config::CONFIG[["rubylibdir"]].sub(Config::CONFIG[["exec_prefix"]], "")' 2>/dev/null`
        RUBY_SITE_PKG=`$RUBY -rrbconfig -e'print Config::CONFIG[["archdir"]].sub(Config::CONFIG[["exec_prefix"]], "")' 2>/dev/null`
    fi
    RUBY_SITE_LIB="\${prefix}/$RUBY_SITE_LIB"
    RUBY_SITE_PKG="\${prefix}/$RUBY_SITE_PKG"
    AC_SUBST([RUBY_SITE_LIB])
    AC_SUBST([RUBY_SITE_PKG]) dnl not strictly needed, ax_ruby_devel already ac_substs this
fi
])
