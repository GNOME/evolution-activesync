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

libeas_la_CPPFLAGS =			\
		$(GLIB_CFLAGS)		\
		$(EDATASERVER_CFLAGS)	\
		$(DBUS_CFLAGS)		\
		$(DBUS_GLIB_CFLAGS)	\
		$(LIBXML_CFLAGS)	\
		$(SOUP_CFLAGS)		\
		$(WBXML_CFLAGS)		\
		$(LIBICAL_CFLAGS)

libeas_la_LDFLAGS =			\
		$(GLIB_LIBS)		\
		$(EDATASERVER_LIBS)	\
		$(DBUS_LIBS)		\
		$(DBUS_GLIB_LIBS)	\
		$(LIBXML_LIBS)		\
		$(SOUP_LIBS)		\
		$(WBXML_LIBS)		\
		$(LIBICAL_LIBS)



libeas_la_SOURCES =		\
        eas-errors.h	\
        eas-connection.c \
        eas-connection.h \
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
        eas-sync-req.h \
        eas-sync-req.c \
        eas-sync-msg.h \
        eas-sync-msg.c \
        eas-email-info-translator.h \
        eas-email-info-translator.c \
        eas-sync-folder-hierarchy-req.h \
        eas-sync-folder-hierarchy-req.c \
        eas-send-email-req.h \
        eas-send-email-req.c \
        eas-send-email-msg.h \
        eas-send-email-msg.c \
        eas-get-email-body-req.h \
        eas-get-email-body-req.c \
        eas-get-email-body-msg.h \
        eas-get-email-body-msg.c \
        eas-cal-info-translator.h \
        eas-cal-info-translator.c \
        eas-delete-email-req.h \
        eas-delete-email-req.c \
        eas-update-email-req.h \
        eas-update-email-req.c \
        eas-get-email-attachment-req.h \
        eas-get-email-attachment-req.c \
        eas-get-email-attachment-msg.h \
        eas-get-email-attachment-msg.c \
        eas-update-calendar-req.h \
        eas-update-calendar-req.c \
        eas-utils.h \
        eas-utils.c \
        eas-add-calendar-req.h \
        eas-add-calendar-req.c \
		eas-connection-errors.c       



#CLEANFILES = $(MARSHAL_GENERATED)

