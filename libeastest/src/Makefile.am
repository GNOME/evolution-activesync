## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES =  libeastest.la

# TODO remove unnecessary includes
libeastest_la_CPPFLAGS =			\
		$(GLIB_CFLAGS)		\
		$(EDATASERVER_CFLAGS)	\
		$(DBUS_GLIB_CFLAGS)
		$(LIBXML_CFLAGS)	\
		$(SOUP_CFLAGS)		\
		-DG_LOG_DOMAIN=\"libeastest\" \
		-I$(top_srcdir)/eas-daemon/libeas \
		$(NULL)

#TODO remove unnecessary libs	  
libeastest_la_LDFLAGS =			\
		$(GLIB_LIBS)		\
		$(EDATASERVER_LIBS)	\
		$(DBUS_GLIB_LIBS)	\
		$(LIBXML_LIBS)	\
		$(SOUP_LIBS)	\
		$(NULL)

libeastest_la_SOURCES = \
	libeastest.h \
	libeastest.c \
	$(NULL)
        
interface_xml =  ../../eas-daemon/src/eas-test.xml

#Rule to generate the binding headers
eas-test-client-stub.h: $(interface_xml)
	dbus-binding-tool --prefix=eas_object --mode=glib-client $< > $@
	       
BUILT_SOURCES = eas-test-client-stub.h
AM_CFLAGS = $(INTI_CFLAGS)
AM_CFLAGS += -g $(WARNING_FLAGS)

libeastest_la_LIBADD = \
	../../libeasclient/libeasclient.la \
	$(top_builddir)/eas-daemon/libeas/libeas.la \
	$(NULL)

# Correctly clean the generated headers, but keep the xml description
CLEANFILES = $(BUILT_SOURCES)
MOSTLYCLEANFILES = libeastest.la
MAINTAINERCLEANFILES = Makefile.in
