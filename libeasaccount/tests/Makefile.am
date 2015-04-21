## Process this file with automake to produce Makefile.in
bin_PROGRAMS = libeasaccount_test

#libexec_PROGRAMS = libeasaccount_test

libeasaccount_test_CPPFLAGS =			\
		$(GLIB_CFLAGS)			\
		$(EDATASERVER_CFLAGS)	\
		$(GCONF_CFLAGS)	\
		$(NULL)

libeasaccount_test_LDFLAGS =			\
		$(GLIB_LIBS)		\
		$(EDATASERVER_LIBS) \
		$(GCONF_LIBS) \
		$(NULL)

AM_CFLAGS = $(INTI_CFLAGS)
AM_CFLAGS += -g $(WARNING_FLAGS)

libeasaccount_test_SOURCES =libeasaccount_test.c

libeasaccount_test_LDADD =../src/libeasaccount.la $(INTI_LIBS)

CLEANFILES =$(BUILT_SOURCES)
