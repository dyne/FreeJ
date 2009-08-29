dnl ===========================================================================
dnl  Macros to save and restore default flags

AC_DEFUN([FREEJ_SAVE_FLAGS], [
  CPPFLAGS_save="${CPPFLAGS}"
  CFLAGS_save="${CFLAGS}"
  CXXFLAGS_save="${CXXFLAGS}"
  OBJCFLAGS_save="${OBJCFLAGS}"
  LDFLAGS_save="${LDFLAGS}"
  LIBS_save="${LIBS}"
])

AC_DEFUN([FREEJ_RESTORE_FLAGS], [
  CPPFLAGS="${CPPFLAGS_save}"
  CFLAGS="${CFLAGS_save}"
  CXXFLAGS="${CXXFLAGS_save}"
  OBJCFLAGS="${OBJCFLAGS_save}"
  LDFLAGS="${LDFLAGS_save}"
  LIBS="${LIBS_save}"
])

dnl ===========================================================================
dnl  Macro to check the presence of a library and its headers
dnl  (better safe than sorry)
dnl
dnl  Usage (all arguments mandatory):
dnl  FREEJ_CHECK_LIB_HEADERS(library, function, headers,
dnl                          action-if-found, action-if-not-found)

AC_DEFUN([FREEJ_CHECK_LIB_HEADERS], [
  AC_CHECK_LIB([$1], [$2], [have_lib_$1=yes], [have_lib_$1=no])
  AC_CHECK_HEADERS([$3], [have_headers_$1=yes], [have_headers_$1=no])
  AS_IF([test "x$have_lib_$1" = "xyes" && test "x$have_headers_$1" = "xyes"],
        [$4], [$5])
])

