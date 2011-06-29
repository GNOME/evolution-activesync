AC_INIT(eas-daemon, 0.9)
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
PKG_CHECK_MODULES(GLIB, glib-2.0, have_glib="yes", have_glib="no")
PKG_CHECK_MODULES(EDATASERVER, libedataserver-1.2)
PKG_CHECK_MODULES(DBUS_GLIB, dbus-glib-1)
PKG_CHECK_MODULES(DBUS, dbus-1)
PKG_CHECK_MODULES(LIBXML, libxml-2.0)
PKG_CHECK_MODULES(SOUP, libsoup-2.4)
PKG_CHECK_MODULES(WBXML, libwbxml2 >= 0.11)
PKG_CHECK_MODULES(LIBICAL, libical)
PKG_CHECK_MODULES(CAMEL, camel-provider-1.2)
PKG_CHECK_MODULES(LIBEBACKEND, libebackend-1.2)
PKG_CHECK_MODULES(SYNCEVOLUTION, syncevolution,
				 have_syncevolution="yes",
				 have_syncevolution="no")
#PKG_CHECK_MODULES(GCONF, gconf-2.0)

if (test "${have_syncevolution}" = "yes"); then
    # Dummy compilation check. If it fails, it's almost certainly because
    # syncevolution.pc specifies a -lsynthesissdk which doesn't exist.
    AC_CHECK_LIB(syncevolution, main, ,
		 [have_syncevolution="no";
                 AC_MSG_NOTICE([Your syncevolution installation is broken (missing -lsynthesissdk?)])],
                 ${SYNCEVOLUTION_LIBS})
fi
if (test "${have_syncevolution}" = "yes"); then
    AC_LANG_PUSH([C++])
    AC_CHECK_HEADER(syncevo/GLibSupport.h, ,
                 [have_syncevolution="no";
                 AC_MSG_NOTICE([Your syncevolution installation lacks GLibSupport.h])],
                 ${SYNCEVOLUTION_CFLAGS})
    AC_LANG_POP([C++])
fi

AM_CONDITIONAL(ENABLE_SYNCEVOLUTION, [test "${have_syncevolution}" = "yes"])
if (test "${have_syncevolution}" = "yes"); then
    AX_BOOST_BASE(1.34)
    AC_DEFINE([ENABLE_SYNCEVOLUTION], [], [Description])

    # If syncevolution.pc has a backenddir variable (as it should), then use it
    # Otherwise, fall back to *its* $libdir/syncevolution/backends
    AC_MSG_CHECKING([where to install SyncEvolution plugin])
    if (test "${syncevo_backenddir}" = ""); then
        syncevo_backenddir=`$PKG_CONFIG --variable=backenddir syncevolution`
        if (test "${syncevo_backenddir}" = ""); then
            syncevo_backenddir=`$PKG_CONFIG --variable=libdir syncevolution`/syncevolution/backends
        fi
    fi
    AC_MSG_RESULT([$syncevo_backenddir])
fi
AC_SUBST(syncevo_backenddir)

# use the "build as module" part of syncevolution/Makefile.am,
# unconditionally when compiling as part of activesyncd
AM_CONDITIONAL([ENABLE_MODULES], [true])

dbus_servicesdir=`$PKG_CONFIG --variable=session_bus_services_dir dbus-1`
AC_SUBST(dbus_servicesdir)

camel_providerdir=`$PKG_CONFIG --variable=camel_providerdir camel-provider-1.2`
AC_SUBST(camel_providerdir)

AC_PATH_PROGS(ASKPASS, [ssh-askpass gnome-ssh-askpass], [ssh-askpass], [path = $PATH:/usr/libexec:/usr/libexec/ssh:/usr/libexec/openssh:$libexecdir:$libexecdir/ssh:$libexecdir/openssh])
AC_SUBST(ASKPASS)

AS_AC_EXPAND(LIBEXECDIR, $libexecdir)

AC_CONFIG_FILES([
    Makefile 
    eas-daemon/data/Makefile 
    eas-daemon/data/org.meego.activesyncd.service
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




AC_OUTPUT
