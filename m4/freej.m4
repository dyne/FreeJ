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

