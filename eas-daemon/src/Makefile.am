## Process this file with automake to produce Makefile.in

libexec_PROGRAMS = activesyncd

BUILT_SOURCES = eas-sync-stub.h eas-common-stub.h eas-mail-stub.h eas-test-stub.h
MARSHAL_GENERATED = eas-marshal.c eas-marshal.h

# generate marshal functions for progress signals:
eas-marshal.h: eas-marshal.list
	$(AM_V_GEN) ( $(GLIB_GENMARSHAL) --prefix=eas_marshal $(srcdir)/eas-marshal.list --header > eas-marshal.tmp \
	&& mv eas-marshal.tmp eas-marshal.h ) \
	|| ( rm -f eas-marshal.tmp && exit 1 )

eas-marshal.c: eas-marshal.h
	$(AM_V_GEN) ( (echo '#include "eas-marshal.h"'; $(GLIB_GENMARSHAL) --prefix=eas_marshal $(srcdir)/eas-marshal.list --body) > eas-marshal.tmp \
	&& mv eas-marshal.tmp eas-marshal.c ) \
	|| ( rm -f eas-marshal.tmp && exit 1  )

eas-mail.c eas-common.c: $(MARSHAL_GENERATED)

eas-sync-stub.h: eas-sync.xml
	dbus-binding-tool --prefix=eas_sync --mode=glib-server $< > $@

eas-common-stub.h: eas-common.xml
	dbus-binding-tool --prefix=eas_common --mode=glib-server $< > $@

eas-mail-stub.h: eas-mail.xml
	dbus-binding-tool --prefix=eas_mail --mode=glib-server $< > $@

eas-test-stub.h: eas-test.xml
	dbus-binding-tool --prefix=eas_test --mode=glib-server $< > $@


activesyncd_CPPFLAGS =			\
		$(GLIB_CFLAGS)		\
		$(EDATASERVER_CFLAGS)	\
		$(DBUS_GLIB_CFLAGS)	\
		$(SOUP_CFLAGS)		\
		-DG_LOG_DOMAIN=\"activesyncd\" \
		-I$(top_srcdir)/eas-daemon/libeas \
		-I$(top_srcdir)/libeasclient \
		$(NULL)


activesyncd_LDFLAGS =			\
		$(GLIB_LIBS)		\
		$(EDATASERVER_LIBS)	\
		$(DBUS_GLIB_LIBS)	\
		$(SOUP_LIBS)		\
		$(NULL)


AM_CFLAGS = $(INTI_CFLAGS)
AM_CFLAGS += $(WARNING_FLAGS)
AM_CFLAGS += -DDISABLE_EAS_DAEMON


activesyncd_SOURCES =  activesyncd-common-defs.h \
		      activesyncd-server.c \
		      eas-sync.h \
		      eas-sync.c \
		      eas-common.h \
		      eas-common.c \
		      eas-mail.h \
		      eas-mail.c \
		      eas-test.h \
		      eas-test.c \
		      eas-marshal.h \
		      eas-marshal.c \
		      eas-interface-base.h \
		      eas-interface-base.c \
		      $(NULL)		      

activesyncd_LDADD = \
    ../libeas/libeas.la \
    ../../libeasclient/libeasclient.la \
    ../../libeastest/src/libeastest.la \
    $(INTI_LIBS) \
    $(NULL)

CLEANFILES = $(BUILT_SOURCES) $(MARSHAL_GENERATED)
EXTRA_DIST = eas-marshal.list eas-sync.xml eas-common.xml eas-mail.xml eas-test.xml
