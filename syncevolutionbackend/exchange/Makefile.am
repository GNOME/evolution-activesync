include $(top_srcdir)/m4-repo/autotroll.mk

AM_CPPFLAGS = -I$(srcdir)/../../ -I$(top_srcdir)/test $(BACKEND_CPPFLAGS)

EXTRA_DIST = configure-sub.in

SYNCSOURCES = synccalexchange.la
MOSTLYCLEANFILES = $(SYNCSOURCES)
if ENABLE_MODULES
backend_LTLIBRARIES = $(SYNCSOURCES)
else
noinst_LTLIBRARIES = $(SYNCSOURCES)
endif

MAINTAINERCLEANFILES = Makefile.in

syncsynccalexchange_la_SOURCES = \
	CalExchangeSource.h \
	CalExchangeSource.cpp
synccalexchange_la_LIBADD = $(CALEXCHANGE_LIBS)
synccalexchange_la_LDFLAGS = -module -avoid-version
synccalexchange_la_CXXFLAGS = $(SYNCEVOLUTION_CXXFLAGS) $(CALEXCHANGE_CFLAGS)
