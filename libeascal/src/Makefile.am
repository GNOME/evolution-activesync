## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES =  libeascal.la

#lrm: TODO remove unnecessary includes
libeascal_la_CPPFLAGS =			\
		$(GLIB_CFLAGS)		\
		$(EDATASERVER_CFLAGS)	\
		$(DBUS_CFLAGS)		\
		$(DBUS_GLIB_CFLAGS)
		$(LIBXML_CFLAGS)	\
		$(SOUP_CFLAGS)

#lrm: TODO remove unnecessary libs	  
libeascal_la_LDFLAGS =			\
		$(GLIB_LIBS)		\
		$(EDATASERVER_LIBS)	\
		$(DBUS_LIBS)		\
		$(DBUS_GLIB_LIBS)	\
		$(LIBXML_LIBS)	\
		$(SOUP_LIBS)

libeascal_la_SOURCES =		\
        libeascal.h			\
        libeascal.c                     \
        eas-cal-info.h                     \
        eas-cal-info.c
        
interface_xml =  ../../eas-daemon/src/eas-calendar.xml

#Rule to generate the binding headers
eas-calendar-client-stub.h: $(interface_xml)
	dbus-binding-tool --prefix=eas_object --mode=glib-client $< > $@
	       
BUILT_SOURCES = eas-calendar-client-stub.h
AM_CFLAGS = $(INTI_CFLAGS)
AM_CFLAGS += -g -Wall -DG_DISABLE_DEPRECATED

# Correctly clean the generated headers, but keep the xml description
CLEANFILES = $(BUILT_SOURCES)
MOSTLYCLEANFILES = libeascal.la
MAINTAINERCLEANFILES = Makefile.in
