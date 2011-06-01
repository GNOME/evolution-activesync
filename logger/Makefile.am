## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES = libeaslogger.la

#lrm: TODO remove unnecessary includes
INCLUDES =  -I/usr/include/glib-2.0 \
            -I/usr/lib/glib-2.0/include \
            -I/usr/include/glib-2.0/glib \
            -I/usr/include/glib-2.0/gio \
            -I/usr/lib/i386-linux-gnu/glib-2.0/include

#lrm: TODO remove unnecessary libs	  
LIBS =  -L/usr/lib \
	-lsoup-2.4 \
	-lpthread \
	-lrt \
	-lglib-2.0 \
    -lgio-2.0 \
    -lgobject-2.0 \
    -lgmodule-2.0 \
	-lxml2 \
	-ledataserver-1.2

libeaslogger_la_SOURCES =	\
    eas-logger.h \
    eas-logger.c

	       
AM_CFLAGS = $(INTI_CFLAGS)
AM_CFLAGS += -g -Wall -DG_DISABLE_DEPRECATED

MOSTLYCLEANFILES = libeaslogger.la
MAINTAINERCLEANFILES = Makefile.in