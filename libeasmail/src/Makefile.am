## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES =  libeasmail.la

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
        utils.h				\
		utils.c
        
interface_xml =  ../../eas-daemon/src/eas-mail.xml

#Rule to generate the binding headers
eas-mail-client-stub.h: $(interface_xml)
	dbus-binding-tool --prefix=eas_object --mode=glib-client $< > $@
	       
BUILT_SOURCES = eas-mail-client-stub.h
AM_CFLAGS = $(INTI_CFLAGS)
AM_CFLAGS += -g -Wall -DG_DISABLE_DEPRECATED

libeasmail_la_LIBADD = ../../logger/libeaslogger.la

# Correctly clean the generated headers, but keep the xml description
CLEANFILES = $(BUILT_SOURCES)
MOSTLYCLEANFILES = libeasmail.la
MAINTAINERCLEANFILES = Makefile.in
