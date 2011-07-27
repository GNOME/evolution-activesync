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
                    eas_autodiscover_tests.c \
                    eas_main_tests.c \
                    $(top_builddir)/libeasmail/src/libeasmail.h \
                    $(top_builddir)/libeassync/src/libeassync.h \
                    $(top_builddir)/libeastest/src/libeastest.h \
                    eas_libeassync_tests.c \
                    eas_libeascon_tests.c \
				    eas_folderhierarchy_tests.c \
				    eas_con_info_translator_test.c \
                    eas_cal_info_translator_test.c \
                    eas_gobjects_unittests.c
                    
check_eas_CFLAGS = @CHECK_CFLAGS@ $(GLIB_CFLAGS)

check_eas_LDADD = $(top_builddir)/libeasmail/src/libeasmail.la \
                  $(top_builddir)/libeassync/src/libeassync.la \
                  $(top_builddir)/libeastest/src/libeastest.la \
                  $(top_builddir)/eas-daemon/libeas/libeas.la \
                  $(top_builddir)/libeasaccount/src/libeasaccount.la \
                  $(top_builddir)/eas-daemon/src/activesyncd-eas-interface-base.o \
                  $(top_builddir)/eas-daemon/src/activesyncd-eas-common.o \
                  $(top_builddir)/eas-daemon/src/activesyncd-eas-mail.o \
                  $(top_builddir)/eas-daemon/src/activesyncd-eas-sync.o \
                  $(top_builddir)/eas-daemon/src/activesyncd-eas-test.o \
                  @CHECK_LIBS@ $(GLIB_LIBS)

#eas_daemon_tests.c eas_daemon_tests.h 
