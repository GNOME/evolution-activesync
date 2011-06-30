AC_INIT(libeasaccount, 0.9)
AC_PREREQ([2.59])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.10 no-define])

# Checks for libraries.

# This macro is defined in check.m4 and tests if check.h and
# libcheck.a are installed in your system. It sets CHECK_CFLAGS and
# CHECK_LIBS accordingly.  
#  AM_PATH_CHECK([MINIMUM-VERSION,
#                [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AM_PATH_CHECK()

AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX

#AC_CONFIG_SRCDIR(src/libeasmail.c)

AC_SUBST(NO_UNDEFINED)

AM_GLIB_GNU_GETTEXT

AM_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])

AC_OUTPUT
