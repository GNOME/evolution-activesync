AC_INIT(eas-daemon, 1.0)
AC_PREREQ([2.59])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.10 no-define])

AS_COMPILER_FLAGS(WARNING_FLAGS,
	"-Wall -Wextra
        -Wno-missing-field-initializers
        -Wno-sign-compare
        -Wno-unused-parameter
	-Werror=pointer-to-int-cast
        -Wdeclaration-after-statement
        -Werror-implicit-function-declaration
        -Wformat-nonliteral -Wformat-security -Winit-self
        -Wmissing-declarations -Wmissing-include-dirs
        -Wmissing-noreturn -Wnested-externs -Wpointer-arith
        -Wundef -Wwrite-strings")
AC_SUBST(WARNING_FLAGS)

CFLAGS="$CFLAGS $WARNING_FLAGS"

# Checks for libraries.

# This macro is defined in check.m4 and tests if check.h and
# libcheck.a are installed in your system. It sets CHECK_CFLAGS and
# CHECK_LIBS accordingly.  
#  AM_PATH_CHECK([MINIMUM-VERSION,
#                [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AM_PATH_CHECK()

GETTEXT_PACKAGE=activesyncd
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Package name for gettext])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX
AM_PROG_CC_C_O

#AC_CONFIG_SRCDIR(src/libeasmail.c)

AC_SUBST(NO_UNDEFINED)

AM_GLIB_GNU_GETTEXT

AM_DISABLE_STATIC
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(GNOMEKEYRING, gnome-keyring-1)
PKG_CHECK_MODULES(GLIB, glib-2.0)
PKG_CHECK_MODULES(EDATASERVER, libedataserver-1.2)
PKG_CHECK_MODULES(DBUS_GLIB, dbus-glib-1)
PKG_CHECK_MODULES(DBUS, dbus-1)
PKG_CHECK_MODULES(LIBXML, libxml-2.0)
PKG_CHECK_MODULES(SOUP, libsoup-2.4)
PKG_CHECK_MODULES(WBXML, libwbxml2 >= 0.11)
PKG_CHECK_MODULES(LIBICAL, libical)
PKG_CHECK_MODULES(CAMEL, camel-provider-1.2)
PKG_CHECK_MODULES(LIBEBACKEND, libebackend-1.2)
PKG_CHECK_MODULES(SYNCEVOLUTION, syncevolution)
#PKG_CHECK_MODULES(GCONF, gconf-2.0)


dbus_servicesdir=`$PKG_CONFIG --variable=session_bus_services_dir dbus-1`
AC_SUBST(dbus_servicesdir)

camel_providerdir=`$PKG_CONFIG --variable=camel_providerdir camel-provider-1.2`
AC_SUBST(camel_providerdir)

# Ick. Shouldn't this be in syncevolution.pc ?
syncevo_backenddir=$libdir/syncevolution/backends
AC_SUBST(syncevo_backenddir)


AC_CONFIG_FILES([
    Makefile 
    eas-daemon/data/Makefile 
    eas-daemon/libeas/Makefile 
    eas-daemon/src/Makefile 
    eas-daemon/tests/Makefile 
    libeasmail/src/Makefile 
    libeassync/src/Makefile 
    check_tests/Makefile
    logger/Makefile
    camel/Makefile
    syncevolution/Makefile
    ])
 	
#activesync account schema installation:
AC_PATH_PROG(GCONFTOOL, gconftool-2)
AM_GCONF_SOURCE_2

AC_ARG_ENABLE(gconf_schemas_install,
[  --enable-gconf-schemas-install    install gconf ActiveSync account schemas],
[case "${enableval}" in
  yes) gconf_schemas_install=true ;;
  no)  gconf_schemas_install=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-gconf-schemas-install) ;;
esac], [gconf_schemas_install=false])
AM_CONDITIONAL(GCONF_SCHEMAS_INSTALL, test x$gconf_schemas_install = xtrue)



AC_OUTPUT
