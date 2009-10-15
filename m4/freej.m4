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
dnl  Macro to check the presence of a library and its header
dnl  (better safe than sorry)
dnl
dnl  Usage (all arguments mandatory):
dnl  FREEJ_CHECK_LIB_HEADER(library, function, header,
dnl                          action-if-found, action-if-not-found)
dnl
dnl  Not using AC_CHECK_LIB and AC_CHECK_HEADERS because they cache
dnl  TODO: use a list of headers instead of a single header (not needed now
dnl        but...)

AC_DEFUN([FREEJ_CHECK_LIB_HEADER], [
  freej_check_lib_headers_save_LIBS=$LIBS
  LIBS="-l$1 $LIBS"
  AC_MSG_CHECKING([for $2 in $LIBS])
  AC_LINK_IFELSE([AC_LANG_PROGRAM([
#ifdef __cplusplus
extern "C"
#endif
char $2 ();], [$2])],
                 [have_lib_$1=yes], [have_lib_$1=no])
  AS_IF([test "x$have_lib_$1" = "xyes"],
        AC_MSG_RESULT([yes]), AC_MSG_RESULT([no]))
  LIBS=$freej_check_lib_headers_save_LIBS
  AC_MSG_CHECKING([for $3 usability])
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([AC_INCLUDES_DEFAULT()
@%:@include <$3>])],
                    [have_header_$1=yes],
                    [have_header_$1=no])
  AS_IF([test "x$have_header_$1" = "xyes"],
        AC_MSG_RESULT([yes]), AC_MSG_RESULT([no]))
  AS_IF([test "x$have_lib_$1" = "xyes" && test "x$have_header_$1" = "xyes"],
        [$4], [$5])
])

