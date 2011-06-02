## Process this file with automake to produce Makefile.in


check_eas_CPPFLAGS =			\
		$(GLIB_CFLAGS)		\
		$(DBUS_CFLAGS)		\
		$(EDATASERVER_CFLAGS)

check_eas_LDFLAGS =			\
		$(GLIB_LIBS)		\
		$(DBUS_LIBS)		\
		$(EDATASERVER_LIBS)



TESTS = check_eas
check_PROGRAMS = check_eas
check_eas_SOURCES = eas_daemon_tests.c \
                    eas_libeasmail_tests.c \
                    eas_libeascal_tests.c \
                    eas_main_tests.c \
                    $(top_builddir)/libeasmail/src/libeasmail.h \
                    $(top_builddir)/libeascal/src/libeascal.h 
check_eas_CFLAGS = @CHECK_CFLAGS@ $(GLIB_CFLAGS)
check_eas_LDADD = $(top_builddir)/libeasmail/src/libeasmail.la \
                        $(top_builddir)/libeascal/src/libeascal.la \
                    @CHECK_LIBS@ $(GLIB_LIBS)

#eas_daemon_tests.c eas_daemon_tests.h 
