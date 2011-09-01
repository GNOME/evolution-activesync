## Process this file with automake to produce Makefile.in
bin_PROGRAMS = libeasmail_test

commondefs = 

INCLUDES = -I/usr/include/glib-2.0 \
	  -I/usr/lib/glib-2.0/include \
          -I/usr/lib/i386-linux-gnu/glib-2.0/include \
          -I/usr/include/evolution-data-server-2.32



LIBS =  -L/usr/lib


BUILT_SOURCES = 

# Correctly clean the generated headers, but keep the xml description
CLEANFILES = $(BUILT_SOURCES)

AM_CFLAGS = $(INTI_CFLAGS) $(WARNING_FLAGS)

libeasmail_test_SOURCES =easmailclient_test.c

libeasmail_test_LDADD = ../src/libeasmail.la $(INTI_LIBS)


