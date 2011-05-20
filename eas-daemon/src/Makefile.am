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

INCLUDES = -I/usr/include/dbus-1.0 \
	  -I/usr/lib/dbus-1.0/include \
	  -I/usr/include/glib-2.0 \
	  -I/usr/lib/glib-2.0/include \
	  -I/usr/include/libsoup-2.4 \
	  -I/usr/include/libxml2

LIBS =  -L/usr/lib \
	-ldbus-1 \
	-lpthread \
	-lrt \
	-lglib-2.0 \
	-ldbus-glib-1 \
	-lgobject-2.0 \
	-lgthread-2.0 


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

activesyncd_LDADD = ../libeas/libeas.la $(INTI_LIBS)

CLEANFILES = $(BUILT_SOURCES)

