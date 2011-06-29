# Yikes, these should be gotten automatically.
libeassync_CFLAGS = \
	-I$(top_srcdir)/libeassync/src \
	$(GLIB_CFLAGS) \
	$(SOUP_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(EDATASERVER_CFLAGS) \
	$(DBUS_GLIB_CFLAGS)
libeassync_LIBADD = \
	$(top_builddir)/libeasmail/src/libeasmail.la	\
	$(top_builddir)/libeassync/src/libeassync.la	\
	$(top_builddir)/eas-daemon/libeas

AM_CPPFLAGS = \
	-DHAVE_CONFIG_H \
	-DHAVE_GLIB \
	-I. \
	$(SYNCEVOLUTION_CFLAGS) \
	$(libeassync_CFLAGS)

backenddir = $(syncevo_backenddir)

SYNCSOURCES = syncactivesync.la
MOSTLYCLEANFILES = $(SYNCSOURCES)
backend_LTLIBRARIES = $(SYNCSOURCES)

MAINTAINERCLEANFILES = Makefile.in

SYNCACTIVESYNC_SOURCES = \
	ActiveSyncSource.h \
	ActiveSyncSource.cpp \
	ActiveSyncSourceRegister.cpp

syncactivesync_la_SOURCES = $(SYNCACTIVESYNC_SOURCES)
syncactivesync_la_LIBADD = $(SYNCEVOLUTION_LIBS) $(libeassync_LIBADD)
syncactivesync_la_LDFLAGS = -no-undefined -module -avoid-version -export-dynamic
syncactivesync_la_CXXFLAGS = $(SYNCEVOLUTION_CXXFLAGS)
