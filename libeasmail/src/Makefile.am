## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES =  libeasmail.la

#lrm: TODO remove unnecessary includes
INCLUDES =  -I/usr/include/libsoup-2.4 \
            -I/usr/include/libxml2 \
            -I/usr/include/glib-2.0 \
            -I/usr/lib/glib-2.0/include \
            -I/usr/include/glib-2.0/glib \
            -I/usr/include/glib-2.0/gio \
            -I/usr/include/evolution-data-server-2.28 \
            -I/usr/include/dbus-1.0 \
            -I/usr/lib/dbus-1.0/include \
            -I/usr/lib/i386-linux-gnu/glib-2.0/include \
            -I/usr/include/evolution-data-server-2.32

 

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
	-ldbus-glib-1 \
	-lxml2 \
	-ledataserver-1.2

libeasmail_la_SOURCES =		\
        libeasmail.h		\
        libeasmail.c		\
        eas-folder.h		\
        eas-folder.c		\
		eas-email-info.h	\
        eas-email-info.c	\
		eas-attachment.h	\
        eas-attachment.c	\
		eas-mail-errors.c	\
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

# Correctly clean the generated headers, but keep the xml description
CLEANFILES = $(BUILT_SOURCES)
MOSTLYCLEANFILES = libeasmail.la
MAINTAINERCLEANFILES = Makefile.in
