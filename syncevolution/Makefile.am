AM_CPPFLAGS = \
	$(SYNCEVOLUTION_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(EASSYNC_CFLAGS) \
        $(BACKEND_CPPFLAGS)

EXTRA_DIST = configure-sub.in ActiveSyncSourceRegister.cpp

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
	ActiveSyncCalendarSource.h \
	ActiveSyncCalendarSource.cpp

syncactivesync_la_SOURCES = $(SYNCACTIVESYNC_SOURCES)
syncactivesync_la_LIBADD = $(SYNCEVOLUTION_LIBS) $(EASSYNC_LIBS)
syncactivesync_la_LDFLAGS = -no-undefined -module -avoid-version -export-dynamic
syncactivesync_la_CXXFLAGS = $(SYNCEVOLUTION_CXXFLAGS)
