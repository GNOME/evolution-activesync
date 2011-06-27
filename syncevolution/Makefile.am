# Yikes, these should be gotten automatically.
libeassync_CFLAGS = \
	-I$(top_srcdir)/libeassync/src \
	$(GLIB_CFLAGS) \
	$(SOUP_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(EDATASERVER_CFLAGS) \
	$(DBUS_GLIB_CFLAGS)

AM_CPPFLAGS = \
	$(SYNCEVOLUTION_CFLAGS) \
	$(libeassync_CFLAGS)

backenddir = $(syncevo_backenddir)

SYNCSOURCES = syncactivesync.la
MOSTLYCLEANFILES = $(SYNCSOURCES)
backend_LTLIBRARIES = $(SYNCSOURCES)

MAINTAINERCLEANFILES = Makefile.in

SYNCACTIVESYNC_SOURCES = \
	ActiveSyncSource.h \
	ActiveSyncSource.cpp

syncactivesync_la_SOURCES = $(SYNCACTIVESYNC_SOURCES)
syncactivesync_la_LIBADD = $(ACTIVESYNC_LIBS) $(SYNCEVOLUTION_LIBS)
syncactivesync_la_LDFLAGS = -module -avoid-version
syncactivesync_la_CXXFLAGS = $(SYNCEVOLUTION_CXXFLAGS)
