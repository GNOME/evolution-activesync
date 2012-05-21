ACLOCAL_AMFLAGS = -I m4 -I m4-repo
#SUBDIRS = libeasmail/tests

if ENABLE_MEEGO_UI
  MEEGO_DIR = meego
endif

if ENABLE_QT_CONFIG_TOOL
  QTCONFIG_DIR = QtActivesyncdConfig
endif

if ENABLE_CAMEL_BACKEND
  CAMEL_DIRS = libevoeas camel
endif

if ENABLE_EPLUGIN
  EPLUGIN_DIR = eplugin
endif

# full set of directories for "make dist"
DIST_SUBDIRS = . libeasaccount/src libeasclient libeasaccount/tests eas-daemon check_tests camel eplugin meego QtActivesyncdConfig po
# subset of that list for "make && make install"
SUBDIRS = . libeasaccount/src libeasclient libeasaccount/tests eas-daemon check_tests $(CAMEL_DIRS) $(EPLUGIN_DIR) $(MEEGO_DIR) $(QTCONFIG_DIR) po

EXTRA_DIST = autogen.sh

pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = libeasaccount.pc libeasclient.pc
