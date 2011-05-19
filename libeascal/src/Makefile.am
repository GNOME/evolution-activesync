## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES =  libeascal.la

#lrm: TODO remove unnecessary includes
INCLUDES =  -I/usr/include/libsoup-2.4 \
            -I/usr/include/libxml2 \
            -I/usr/include/glib-2.0 \
            -I/usr/lib/glib-2.0/include \
            -I/usr/include/glib-2.0/glib \
            -I/usr/include/glib-2.0/gio \
            -I/usr/include/evolution-data-server-2.28 \
            -I/usr/include/dbus-1.0 \
            -I/usr/lib/dbus-1.0/include 

#lrm: TODO remove unnecessary libs	  
LIBS =  -L/usr/lib \
	-lsoup-2.4 \
	-lpthread \
	-lrt \
	-lglib-2.0 \
    -lgio-2.0 \
    -lgobject-2.0 \
    -lgmodule-2.0 \
	-ldbus-1 \
	-ldbus-glib-1

libeascal_la_SOURCES =		\
        libeascal.h			\
        libeascal.c	
        
interface_xml =  ../../eas-daemon/libeas/eas-daemon-interface.xml

#Rule to generate the binding headers
eas-mail-client-stub.h: $(interface_xml)
	dbus-binding-tool --prefix=eas_object --mode=glib-client $< > $@
	       
BUILT_SOURCES = eas-mail-client-stub.h

# Correctly clean the generated headers, but keep the xml description
CLEANFILES = $(BUILT_SOURCES)
MOSTLYCLEANFILES = libeascal.la
MAINTAINERCLEANFILES = Makefile.in
