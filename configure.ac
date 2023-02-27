AC_INIT(evolution-activesync, 0.92)
AM_MAINTAINER_MODE([enable])
AC_PREREQ([2.59])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.10 no-define dist-xz no-dist-gzip])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for libraries.

# find compile flags for the "check" C++ unit testing framework
PKG_CHECK_MODULES(CHECK, check)

GETTEXT_PACKAGE=activesyncd
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Package name for gettext])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX
AM_PROG_CC_C_O

DK_ARG_ENABLE_WARNINGS([WARNING_FLAGS],
        [-Wall],
        [-Wall
         -Wextra
         -Wno-missing-field-initializers
         -Wno-sign-compare
         -Wno-unused-parameter
         -Werror=pointer-to-int-cast
         -Wdeclaration-after-statement
         -Werror-implicit-function-declaration
         -Wformat-nonliteral
         -Wformat-security
         -Winit-self
         -Wmissing-declarations
         -Wmissing-include-dirs
         -Wmissing-noreturn
         -Wnested-externs
         -Wpointer-arith
         -Wundef
         -Wwrite-strings],
        [G])


#AC_CONFIG_SRCDIR(src/libeasmail.c)

AC_SUBST(NO_UNDEFINED)

IT_PROG_INTLTOOL([0.35.5])
AM_GLIB_GNU_GETTEXT
AM_PATH_GLIB_2_0([2.32.0], [], [AC_ERROR([glib 2.32 not found])])
AM_DISABLE_STATIC
AC_PROG_LIBTOOL

privincludedir='${includedir}'/eas-daemon
AC_SUBST(privincludedir)

PKG_CHECK_MODULES(LIBSECRET, libsecret-1)
PKG_CHECK_MODULES(EDATASERVER, libedataserver-1.2)
PKG_CHECK_MODULES(DBUS_GLIB, dbus-glib-1)
PKG_CHECK_MODULES(LIBXML, libxml-2.0)
PKG_CHECK_MODULES(SOUP, libsoup-2.4)
PKG_CHECK_MODULES(WBXML, libwbxml2 >= 0.11)
PKG_CHECK_MODULES(LIBICAL, libical)
PKG_CHECK_MODULES(LIBEBOOK, libebook-1.2)
PKG_CHECK_MODULES(LIBEBACKEND, libebackend-1.2)
PKG_CHECK_MODULES(GIO, gio-2.0)
PKG_CHECK_MODULES(LIBEMAIL_ENGINE, libemail-engine, [have_libemail_engine="yes"], [have_libemail_engine="no"])

EVO_PKG="-3.0"
CAMEL_PKG="camel-1.2"

PKG_CHECK_MODULES(CAMEL, ${CAMEL_PKG} >= 3.23.2, [have_camel=yes], [have_camel=no])

AC_SUBST(EVOLUTION_SHELL_CFLAGS)
AC_SUBST(EVOLUTION_SHELL_LIBS)

PKG_CHECK_MODULES(EVOLUTION_MAIL, evolution-mail${EVO_PKG}, [have_e_mail=yes], [have_e_mail=no])
AC_SUBST(EVOLUTION_MAIL_CFLAGS)
AC_SUBST(EVOLUTION_MAIL_LIBS)

PKG_CHECK_MODULES(EVOLUTION_SHELL, evolution-shell${EVO_PKG}, [have_e_shell=yes], [have_e_shell=no])
AC_SUBST(EVOLUTION_SHELL_CFLAGS)
AC_SUBST(EVOLUTION_SHELL_LIBS)

# Check for the MeeGo "mlite" library.  If it exists, enable the MeeGo
# ActiveSync UI.
PKG_CHECK_MODULES([MLITE],
                  [mlite],
		  [have_mlite="yes"],
		  [have_mlite="no"
                   AC_MSG_NOTICE([MeeGo ActiveSync Settings UI will not be built])])

camel_providerdir=`$PKG_CONFIG --variable=camel_providerdir ${CAMEL_PKG}`
AC_SUBST(camel_providerdir)

if test "$have_e_plugin" = "yes"; then
   plugindir="`$PKG_CONFIG --variable=plugindir evolution-plugin${EVO_PKG}`"
   AC_SUBST(plugindir)

   EVOLUTION_PLUGIN_errordir="`$PKG_CONFIG --variable=errordir evolution-plugin${EVO_PKG}`"
   AC_SUBST(EVOLUTION_PLUGIN_errordir)
fi

if test "$have_e_shell" = "yes"; then
   EVOLUTION_imagesdir="`$PKG_CONFIG --variable=imagesdir evolution-shell${EVO_PKG}`"
   AC_SUBST(EVOLUTION_imagesdir)
fi

EVOLUTION_privlibdir=`$PKG_CONFIG --variable=privlibdir evolution-data-server-1.2`
AC_SUBST(EVOLUTION_privlibdir)

eds_moduledir=`$PKG_CONFIG --variable=moduledir libebackend-1.2`
AC_SUBST(eds_moduledir)

evo_moduledir=`$PKG_CONFIG --variable=moduledir evolution-shell${EVO_PKG}`
AC_SUBST(evo_moduledir)

AC_PATH_PROGS(ASKPASS, [ssh-askpass gnome-ssh-askpass], [ssh-askpass], [path = $PATH:/usr/libexec:/usr/libexec/ssh:/usr/libexec/openssh:$libexecdir:$libexecdir/ssh:$libexecdir/openssh])
AC_SUBST(ASKPASS)

MEEGO_UI_DIR=meego/meego-ux-settings-activesync
AM_CONDITIONAL([ENABLE_MEEGO_UI],[test "$have_mlite" = yes])

GLIB_GSETTINGS

AC_ARG_ENABLE(qtconfig,
              AS_HELP_STRING([--enable-qtconfig],
                             [Compile the (obsolete) Qt configuration UI]),
              [enable_qtconfig="$enableval"], [enable_qtconfig="no"])

if test "$enable_qtconfig" = "yes"; then
   AT_WITH_QT(,no_keywords,,[have_qt=no],[have_qt=yes])
   if (test "$have_qt" = "no"); then
      AC_MSG_NOTICE([Qt build environment not found; will not build QtActiveSyncdConfig tool])
      enable_qtconfig=no
   fi
fi
AM_CONDITIONAL([ENABLE_QT_CONFIG_TOOL],[test "$enable_qtconfig" = "yes"])

AC_ARG_ENABLE(camel-backend,
              AS_HELP_STRING([--disable-camel-backend],
                             [Can be used to disable compiling the libcamel email backend. Enabled by default.]),
              [enable_camel="$enableval"], [enable_camel="yes"])
AM_CONDITIONAL([ENABLE_CAMEL_BACKEND], [test "$enable_camel" = "yes"])
if test "$enable_camel" = "yes" && (test "$have_libemail_engine" != "yes" || test "$have_camel" != "yes"); then
    AC_MSG_ERROR(["libemail-engine.pc and recent enough camel-1.2.pc are required for the camel backend; use --disable-camel-backend when EDS is too old or install a more recent EDS"])
fi

AS_AC_EXPAND(LIBEXECDIR, $libexecdir)

AC_CONFIG_FILES([
    Makefile
    eas-daemon/Makefile
    eas-daemon/data/Makefile
    eas-daemon/data/org.meego.activesyncd.service
    eas-daemon/libeas/Makefile
    eas-daemon/src/Makefile
    collection/Makefile
    configuration/Makefile
    libeastest/src/Makefile
    libeasclient/Makefile
    libeasaccount/src/Makefile
    libeasaccount/tests/Makefile
    check_tests/Makefile
    camel/Makefile
    libevoeas/Makefile
    meego/Makefile
    QtActivesyncdConfig/Makefile
    po/Makefile.in
    libeasaccount.pc
    libeasclient.pc
    ])

AC_CONFIG_COMMANDS_POST([test "$have_mlite" = "yes" && pushd ${srcdir}/${MEEGO_UI_DIR} && qmake && popd])

AC_OUTPUT
