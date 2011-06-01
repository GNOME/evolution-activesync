## Process this file with automake to produce Makefile.in

bin_PROGRAMS = activesyncd

BUILT_SOURCES = eas-calendar-stub.h eas-common-stub.h eas-contact-stub.h eas-mail-stub.h

eas-calendar-stub.h: eas-calendar.xml
	dbus-binding-tool --prefix=eas_calendar --mode=glib-server $< > $@


eas-common-stub.h: eas-common.xml
	dbus-binding-tool --prefix=eas_common --mode=glib-server $< > $@


eas-contact-stub.h: eas-contact.xml
	dbus-binding-tool --prefix=eas_contact --mode=glib-server $< > $@

eas-mail-stub.h: eas-mail.xml
	dbus-binding-tool --prefix=eas_mail --mode=glib-server $< > $@

INCLUDES = -I../libeas \
	  -I../../libeasmail/src

activesyncd_CPPFLAGS =			\
		$(GLIB_CFLAGS)		\
		$(EDATASERVER_CFLAGS)	\
		$(DBUS_CFLAGS)		\
		$(DBUS_GLIB_CFLAGS)	\
		$(SOUP_CFLAGS)

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
		      eas-calendar.h \
		      eas-calendar.c \
		      eas-common.h \
		      eas-common.c \
		      eas-contact.h \
		      eas-contact.c \
		      eas-mail.h \
		      eas-mail.c
		      

activesyncd_LDADD = ../libeas/libeas.la ../../libeasmail/src/libeasmail.la ../../libeascal/src/libeascal.la $(INTI_LIBS)

CLEANFILES = $(BUILT_SOURCES)

