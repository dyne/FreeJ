# AC_AVIFILE 0.1.0
# CXXFLAGS and LIBS for avifile
# taken from Autostar Sandbox, http://autostars.sourceforge.net/
# constructed by careful cross-pollination from various sources and lots of
# hard labour

dnl Usage:
dnl AC_AVIFILE(MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for avifile, and defines 
dnl - AVIFILE_CXXFLAGS (C++ compiler flags)
dnl - AVIFILE_LDFLAGS (linker flags, stripping and path)
dnl as well as set HAVE_AVIFILE to yes or no
dnl
dnl FIXME: should define AVIFILE_VERSION
dnl
dnl prerequisites: 
dnl - working C++ compiler
dnl - usable libstdc++
dnl - AC_PATH_XTRA

AC_DEFUN(AC_AVIFILE, 
[
  HAVE_AVIFILE="no"

  dnl first look for avifile-config
  AC_PATH_PROG(AVIFILE_CONFIG, avifile-config, no)
  min_avifile_version=ifelse([$1], ,0.7.0,$1)
  if test "x$AVIFILE_CONFIG" != "xno"; then
    dnl now that we have it, we need to save libs and cflags
    AVIFILE_LDFLAGS=`avifile-config --libs`
    AVIFILE_CXXFLAGS=`avifile-config --cflags`
    AC_SUBST(AVIFILE_LDFLAGS)
    AC_SUBST(AVIFILE_CXXFLAGS)
    HAVE_AVIFILE="yes"
  fi

  dnl we got this far, now start checking if we have the right version
  if test "x$HAVE_AVIFILE" = "xyes";
  then
    AC_MSG_CHECKING(for avifile - version >= $min_avifile_version)
    dnl first get the version number from avifile-config
    avifile_major_version=`$AVIFILE_CONFIG $avifile_args --version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    avifile_minor_version=`$AVIFILE_CONFIG $avifile_args --version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    avifile_micro_version=`$AVIFILE_CONFIG $avifile_config_args --version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    dnl now run a short C app that tells us if the version is ok or not
    dnl all of this is boilerplate code from other examples
    rm -f conf.avifiletest
    AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main ()
{
  int major, minor, micro;
  char ver[50];

  system ("touch conf.avifiletest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  strncpy(ver, "$min_avifile_version", sizeof(ver) - 1);
  if (sscanf(ver, "%d.%d.%d", &major, &minor, &micro) != 3) 
  {
    printf ("%s, bad version string\n", "$min_avifile_version");
    exit (1);
  }
  if (($avifile_major_version > major) ||
     (($avifile_major_version == major) && ($avifile_minor_version > minor)) || 
     (($avifile_major_version == major) && ($avifile_minor_version == minor) && ($avifile_micro_version >= micro)))
    return 0;
  else
  {
    printf ("\n*** 'avifile-config --version' returned %d.%d.%d, but the minimum version\n", $avifile_major_version, $avifile_minor_version, $avifile_micro_version); 
    printf ("*** of AVIFILE required is %d.%d.%d. If avifile-config is correct, then it is\n", major, minor, micro);
    printf ("*** best to upgrade to the required version.\n");
    printf ("*** If avifile-config was wrong, set the environment variable AVIFILE_CONFIG\n");
    printf ("*** to point to the correct copy of avifile-config, and remove the file\n");
    printf ("*** config.cache (if it exists) before re-running configure\n");
    return 1;
  }
}
    ], 
    [ 
      AC_MSG_RESULT(yes)
      HAVE_AVIFILE="yes"
    ],
    [ 
      HAVE_AVIFILE="no"
    ])
  fi
  if test "x$HAVE_AVIFILE" = "xyes"; then
    dnl add X flags, because it looks like avifile needs libXv and libXxf86vm
    CXXFLAGS="$XXCFLAGS $AVIFILE_CXXFLAGS" 
#   commented and modified by jaromil 27 may 2003 pescara ecoteca mo'
#   AVIFILE_LIBS="$AVIFILE_LDFLAGS $X_LIBS -lXv -lXxf86vm -ldivxdecore"
    AVIFILE_LIBS="-laviplay -lXv -lXxf86vm"
  fi
  AC_SUBST(HAVE_AVIFILE)
])

