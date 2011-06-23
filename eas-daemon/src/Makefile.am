## Process this file with automake to produce Makefile.in

bin_PROGRAMS = activesyncd

BUILT_SOURCES = eas-sync-stub.h eas-common-stub.h eas-mail-stub.h

eas-sync-stub.h: eas-sync.xml
	dbus-binding-tool --prefix=eas_sync --mode=glib-server $< > $@


eas-common-stub.h: eas-common.xml
	dbus-binding-tool --prefix=eas_common --mode=glib-server $< > $@

eas-mail-stub.h: eas-mail.xml
	dbus-binding-tool --prefix=eas_mail --mode=glib-server $< > $@

activesyncd_CPPFLAGS =			\
		$(GLIB_CFLAGS)		\
		$(EDATASERVER_CFLAGS)	\
		$(DBUS_CFLAGS)		\
		$(DBUS_GLIB_CFLAGS)	\
		$(SOUP_CFLAGS)		\
		-DG_LOG_DOMAIN=\"activesyncd\" \
		-I$(top_srcdir)/eas-daemon/libeas \
		-I$(top_srcdir)/libeasmail/src


activesyncd_LDFLAGS =			\
		$(GLIB_LIBS)		\
		$(EDATASERVER_LIBS)	\
		$(DBUS_LIBS)		\
		$(DBUS_GLIB_LIBS)	\
		$(SOUP_LIBS)


AM_CFLAGS = $(INTI_CFLAGS)
AM_CFLAGS += -g -Wall -DG_DISABLE_DEPRECATED
AM_CFLAGS += -DDISABLE_EAS_DAEMON


activesyncd_SOURCES =  activesyncd-common-defs.h \
		      activesyncd-server.c \
		      eas-sync.h \
		      eas-sync.c \
		      eas-common.h \
		      eas-common.c \
		      eas-mail.h \
		      eas-mail.c
		      

activesyncd_LDADD = ../../logger/libeaslogger.la ../libeas/libeas.la ../../libeasmail/src/libeasmail.la ../../libeassync/src/libeassync.la $(INTI_LIBS)

CLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST = eas-sync.xml eas-common.xml eas-mail.xml
