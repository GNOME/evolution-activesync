## Process this file with automake to produce Makefile.in
INCLUDES = -I/usr/include/dbus-1.0 \
            -I/usr/include/libxml2 \
	  -I/usr/lib/dbus-1.0/include \
	  -I/usr/include/glib-2.0 \
	  -I/usr/lib/glib-2.0/include \
          -I/usr/lib/i386-linux-gnu/glib-2.0/include \
          -I/usr/include/evolution-data-server-2.32

LIBS =  -L/usr/lib \
	-ldbus-1 \
	-lpthread \
	-lrt \
	-lglib-2.0 \
	-ldbus-glib-1 \
	-lgobject-2.0 \
	-lgthread-2.0 



TESTS = check_eas
check_PROGRAMS = check_eas
check_eas_SOURCES = eas_daemon_tests.c \
                    eas_libeasmail_tests.c \
                    eas_libeascal_tests.c \
                    eas_main_tests.c \
                    $(top_builddir)/libeasmail/src/libeasmail.h \
                    $(top_builddir)/libeascal/src/libeascal.h 
check_eas_CFLAGS = @CHECK_CFLAGS@
check_eas_LDADD = $(top_builddir)/libeasmail/src/libeasmail.la \
                        $(top_builddir)/libeascal/src/libeascal.la \
                        $(top_builddir)/eas-daemon/libeas/libeas.la \
                    @CHECK_LIBS@

#eas_daemon_tests.c eas_daemon_tests.h 
