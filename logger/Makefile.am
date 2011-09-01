## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES = libeaslogger.la

#TODO remove unnecessary includes
libeaslogger_la_CFLAGS =		\
                $(WARNING_FLAGS) \
		$(GLIB_CFLAGS)

#TODO remove unnecessary libs	  
libeaslogger_la_LDFLAGS =		\
		$(GLIB_LIBS)

libeaslogger_la_SOURCES =	\
    eas-logger.h \
    eas-logger.c

	       
AM_CFLAGS = $(INTI_CFLAGS)
AM_CFLAGS += -g $(WARNING_FLAGS)

MOSTLYCLEANFILES = libeaslogger.la
MAINTAINERCLEANFILES = Makefile.in
