AC_DEFUN([BIND_JAVA_SCRIPTING],
[

java_enable=no
java=no

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