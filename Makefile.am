ACLOCAL_AMFLAGS = -I m4 -I m4-repo
#SUBDIRS = libeasmail/tests

if ENABLE_MEEGO_UI
  MEEGO_DIR = meego
endif

if ENABLE_QT_CONFIG_TOOL
  QTCONFIG_DIR = QtActivesyncdConfig
endif

SUBDIRS = . libeasaccount/src libeasclient libeasaccount/tests eas-daemon check_tests camel eplugin $(MEEGO_DIR) $(QTCONFIG_DIR) po

EXTRA_DIST = autogen.sh

pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = libeasaccount.pc libeasclient.pc
