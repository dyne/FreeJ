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