dnl
dnl $Id$ $Name$
dnl
AC_DEFUN([AC_TRY_CXXFLAGS],
	[AC_MSG_CHECKING([if $CXX supports $3 flags])
	SAVE_CXXFLAGS="$CXXFLAGS"
	CXXFLAGS="$3"
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[$1]], [[$2]])],[ac_cv_try_cxxflags_ok=yes], [ac_cv_try_cxxflags_ok=no])
	CXXFLAGS="$SAVE_CXXFLAGS"
	AC_MSG_RESULT([$ac_cv_try_cxxflags_ok])
	if test x"$ac_cv_try_cxxflags_ok" = x"yes"; then
	ifelse([$4],[],[:],[$4])
    else
	ifelse([$5],[],[:],[$5])
    fi])

