## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES = libeas.la

# MARSHAL_GENERATED = eas-marshal.c eas-marshal.h

#eas-marshal.h: eas-marshal.list
#	$(AM_V_GEN) ( $(GLIB_GENMARSHAL) --prefix=eas_marshal $(srcdir)/eas-marshal.list --header > eas-marshal.tmp \
#	&& mv eas-marshal.tmp eas-marshal.h ) \
#	|| ( rm -f eas-marshal.tmp && exit 1 )

#eas-marshal.c: eas-marshal.h
#	$(AM_V_GEN) ( (echo '#include "eas-marshal.h"'; $(GLIB_GENMARSHAL) --prefix=eas_marshal $(srcdir)/eas-marshal.list --body) > eas-marshal.tmp \
#	&& mv eas-marshal.tmp eas-marshal.c ) \
#	|| ( rm -f eas-marshal.tmp && exit 1 )

#eas-connection.c: $(MARSHAL_GENERATED)

AM_CXXFLAGS = $(INTI_CXXFLAGS)

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
	-lwbxml2 \
	-ledataserver-1.2

#        eas-marshal.h
#        eas-marshal.c

libeas_la_SOURCES =		\
        eas-errors.h			\
        eas-errors.c			\
        eas-connection.c \
        eas-connection.h \
        eas-msg.c \
        eas-msg.h \
        eas-msg-base.h \
        eas-msg-base.c \
        eas-sync-folder-msg.h \
        eas-sync-folder-msg.c \
        eas-provision-msg.h \
        eas-provision-msg.c \
        eas-accounts.h \
        eas-accounts.c \
        eas-provision-req.h \
        eas-provision-req.c \
        eas-request-base.h \
        eas-request-base.c \
        eas-sync-folder-hierarchy.h \
        eas-sync-folder-hierarchy.c \
        eas-sync-req.h \
        eas-sync-req.c \
        eas-sync-msg.h \
        eas-sync-msg.c \
        eas-email-info-translator.h \
        eas-email-info-translator.c \
        eas-cal-info-translator.h \
        eas-cal-info-translator.c


#CLEANFILES = $(MARSHAL_GENERATED)

