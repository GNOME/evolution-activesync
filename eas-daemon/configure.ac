AC_INIT(eas-deamon, 1.0)
AC_PREREQ([2.59])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.10 no-define])

AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX

AC_CONFIG_SRCDIR(src/activesyncd-server.c)

AC_SUBST(NO_UNDEFINED)

AM_GLIB_GNU_GETTEXT

AM_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_CONFIG_FILES([Makefile data/Makefile libeas/Makefile src/Makefile tests/Makefile])

AC_OUTPUT

