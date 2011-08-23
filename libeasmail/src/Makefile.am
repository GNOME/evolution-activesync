## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES =  libeasmail.la


MARSHAL_GENERATED = eas-marshal.c eas-marshal.h

# TODO - this doesn't always work; sometimes the marshal files aren't created until these commands are run manually:
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

# TODO remove unnecessary includes
libeasmail_la_CPPFLAGS =		\
		$(GLIB_CFLAGS)		\
		$(EDATASERVER_CFLAGS)	\
		$(DBUS_CFLAGS)		\
		$(DBUS_GLIB_CFLAGS)	\
		$(LIBXML_CFLAGS)	\
		$(SOUP_CFLAGS)		\
		-DG_LOG_DOMAIN=\"libeasmail\"

# TODO remove unnecessary libs	  
libeasmail_la_LDFLAGS =			\
		$(GLIB_LIBS)		\
		$(EDATASERVER_LIBS)	\
		$(DBUS_LIBS)		\
		$(DBUS_GLIB_LIBS)	\
		$(LIBXML_LIBS)	\
		$(SOUP_LIBS)

libeasmail_la_SOURCES =		\
        libeasmail.h		\
        libeasmail.c		\
        eas-folder.h		\
        eas-folder.c		\
		eas-email-info.h	\
        eas-email-info.c	\
		eas-attachment.h	\
        eas-attachment.c	\
		eas-mail-errors.h	\
		eas-marshal.h 		\
		eas-marshal.c           \
		eas-provision-list.h           \
		eas-provision-list.c

libeasmailincludedir = $(privincludedir)/eas-mail
libeasmailinclude_HEADERS = eas-attachment.h eas-folder.h libeasmail.h \
			    eas-email-info.h eas-mail-errors.h


mail_interface_xml =  ../../eas-daemon/src/eas-mail.xml
common_interface_xml = ../../eas-daemon/src/eas-common.xml

#Rules to generate the binding headers
eas-mail-client-stub.h: $(mail_interface_xml)
	dbus-binding-tool --prefix=eas_object --mode=glib-client $< > $@
eas-common-client-stub.h: $(common_interface_xml)	
	dbus-binding-tool --prefix=eas_common --mode=glib-client $< > $@
	       
EXTRA_DIST = eas-marshal.list
BUILT_SOURCES = eas-mail-client-stub.h eas-common-client-stub.h
AM_CFLAGS = $(INTI_CFLAGS)
AM_CFLAGS += -g -Wall -DG_DISABLE_DEPRECATED

libeasmail_la_LIBADD = ../../logger/libeaslogger.la

# Correctly clean the generated headers, but keep the xml description
CLEANFILES = $(BUILT_SOURCES) $(MARSHAL_GENERATED)
MOSTLYCLEANFILES = libeasmail.la
MAINTAINERCLEANFILES = Makefile.in
