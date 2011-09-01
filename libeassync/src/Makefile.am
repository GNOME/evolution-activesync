## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES =  libeassync.la

# TODO remove unnecessary includes
libeassync_la_CPPFLAGS =			\
		$(GLIB_CFLAGS)		\
		$(EDATASERVER_CFLAGS)	\
		$(DBUS_CFLAGS)		\
		$(DBUS_GLIB_CFLAGS)
		$(LIBXML_CFLAGS)	\
		$(SOUP_CFLAGS)		\
		-DG_LOG_DOMAIN=\"libeassync\" \
		-I$(top_srcdir)/eas-daemon/libeas

#TODO remove unnecessary libs	  
libeassync_la_LDFLAGS =			\
		$(GLIB_LIBS)		\
		$(EDATASERVER_LIBS)	\
		$(DBUS_LIBS)		\
		$(DBUS_GLIB_LIBS)	\
		$(LIBXML_LIBS)	\
		$(SOUP_LIBS)

libeassync_la_SOURCES =		\
	eas-sync-errors.h	\
        libeassync.h			\
        libeassync.c                     \
        eas-item-info.h                     \
        eas-item-info.c

libeassyncincludedir = $(privincludedir)/eas-sync
libeassyncinclude_HEADERS = eas-item-info.h eas-sync-errors.h libeassync.h

interface_xml =  ../../eas-daemon/src/eas-sync.xml

#Rule to generate the binding headers
eas-sync-client-stub.h: $(interface_xml)
	dbus-binding-tool --prefix=eas_object --mode=glib-client $< > $@
	       
BUILT_SOURCES = eas-sync-client-stub.h
AM_CFLAGS = $(INTI_CFLAGS)
AM_CFLAGS += -g $(WARNING_FLAGS)

# We need libeasmail.la for eas_folder_serialise()
libeassync_la_LIBADD = \
	../../logger/libeaslogger.la ../../libeasmail/src/libeasmail.la

# Correctly clean the generated headers, but keep the xml description
CLEANFILES = $(BUILT_SOURCES)
MOSTLYCLEANFILES = libeassync.la
MAINTAINERCLEANFILES = Makefile.in
