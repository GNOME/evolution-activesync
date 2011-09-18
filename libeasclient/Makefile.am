## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES =  libeasclient.la

MARSHAL_GENERATED = eas-marshal.c eas-marshal.h

# generate marshal functions for progress signals:
eas-marshal.h: eas-marshal.list
	$(AM_V_GEN) ( $(GLIB_GENMARSHAL) --prefix=eas_marshal $(srcdir)/eas-marshal.list --header > eas-marshal.tmp \
	&& mv eas-marshal.tmp eas-marshal.h ) \
	|| ( rm -f eas-marshal.tmp && exit 1 )

eas-marshal.c: eas-marshal.h
	$(AM_V_GEN) ( (echo '#include "eas-marshal.h"'; $(GLIB_GENMARSHAL) --prefix=eas_marshal $(srcdir)/eas-marshal.list --body) > eas-marshal.tmp \
	&& mv eas-marshal.tmp eas-marshal.c ) \
	|| ( rm -f eas-marshal.tmp && exit 1  )

libeasmail.c: $(MARSHAL_GENERATED)

libeasclient_la_CPPFLAGS =		\
		$(GLIB_CFLAGS)		\
		$(EDATASERVER_CFLAGS)	\
		$(DBUS_CFLAGS)		\
		$(DBUS_GLIB_CFLAGS)	\
		$(LIBXML_CFLAGS)	\
		$(SOUP_CFLAGS)		\
		-DG_LOG_DOMAIN=\"libeasclient\"

libeasclient_la_LDFLAGS =		\
		$(GLIB_LIBS)		\
		$(EDATASERVER_LIBS)	\
		$(DBUS_LIBS)		\
		$(DBUS_GLIB_LIBS)	\
		$(LIBXML_LIBS)		\
		$(SOUP_LIBS)

libeasclient_la_SOURCES =		\
	eas-sync-errors.h		\
        libeassync.h			\
        libeassync.c			\
        eas-item-info.h			\
        eas-item-info.c			\
        libeasmail.h			\
        libeasmail.c			\
        eas-folder.h			\
        eas-folder.c			\
	eas-email-info.h		\
        eas-email-info.c		\
	eas-attachment.h		\
        eas-attachment.c		\
	eas-mail-errors.h		\
	eas-marshal.h			\
	eas-marshal.c			\
	eas-provision-list.h		\
	eas-provision-list.c		\
	eas-logger.c			\
	eas-logger.h

libeasclientincludedir = $(privincludedir)/eas-client
libeasclientinclude_HEADERS = eas-item-info.h eas-sync-errors.h libeassync.h \
			eas-attachment.h eas-folder.h libeasmail.h \
			eas-email-info.h eas-mail-errors.h


mail_interface_xml =  ../eas-daemon/src/eas-mail.xml
common_interface_xml = ../eas-daemon/src/eas-common.xml
sync_interface_xml =  ../eas-daemon/src/eas-sync.xml

#Rule to generate the binding headers
eas-sync-client-stub.h: $(sync_interface_xml)
	dbus-binding-tool --prefix=eas_object --mode=glib-client $< > $@
eas-mail-client-stub.h: $(mail_interface_xml)
	dbus-binding-tool --prefix=eas_object --mode=glib-client $< > $@
eas-common-client-stub.h: $(common_interface_xml)	
	dbus-binding-tool --prefix=eas_common --mode=glib-client $< > $@

EXTRA_DIST = eas-marshal.list
BUILT_SOURCES = eas-sync-client-stub.h eas-mail-client-stub.h eas-common-client-stub.h
AM_CFLAGS = $(INTI_CFLAGS)
AM_CFLAGS += -g $(WARNING_FLAGS)

# Correctly clean the generated headers, but keep the xml description
CLEANFILES = $(BUILT_SOURCES) $(MARSHAL_GENERATED)
MOSTLYCLEANFILES = libeasclient.la
MAINTAINERCLEANFILES = Makefile.in
