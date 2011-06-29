
 ifeq ($(LIBEASSYNC_CFLAGS),)
# When building inside SyncEvolution, these will come from pkgconfig.
# In the activesyncd repository, hard-code them.
LIBEASSYNC_CFLAGS = \
	-I$(top_srcdir)/libeassync/src \
	$(GLIB_CFLAGS) \
	$(SOUP_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(EDATASERVER_CFLAGS) \
	$(DBUS_GLIB_CFLAGS)

LIBEASSYNC_LIBS = $(top_builddir)/libeassync/src/libeassync.la
 endif

AM_CPPFLAGS = \
	-DHAVE_CONFIG_H \
	-DHAVE_GLIB \
	-I. \
	$(SYNCEVOLUTION_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBEASSYNC_CFLAGS)

backenddir = $(syncevo_backenddir)

SYNCSOURCES = syncactivesync.la
MOSTLYCLEANFILES = $(SYNCSOURCES)
if ENABLE_MODULES
backend_LTLIBRARIES = $(SYNCSOURCES)
else
noinst_LTLIBRARIES = $(SYNCSOURCES)
endif

MAINTAINERCLEANFILES = Makefile.in

SYNCACTIVESYNC_SOURCES = \
	ActiveSyncSource.h \
	ActiveSyncSource.cpp \
	ActiveSyncSourceRegister.cpp

syncactivesync_la_SOURCES = $(SYNCACTIVESYNC_SOURCES)
syncactivesync_la_LIBADD = $(SYNCEVOLUTION_LIBS) $(LIBEASSYNC_LIBS)
syncactivesync_la_LDFLAGS = -no-undefined -module -avoid-version -export-dynamic
syncactivesync_la_CXXFLAGS = $(SYNCEVOLUTION_CXXFLAGS)
