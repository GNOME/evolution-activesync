## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES = libeas.la

AM_CXXFLAGS = $(INTI_CXXFLAGS)


libeas_la_CPPFLAGS =			\
		$(GLIB_CFLAGS)		\
		$(EDATASERVER_CFLAGS)	\
		$(DBUS_CFLAGS)		\
		$(DBUS_GLIB_CFLAGS)	\
		$(LIBXML_CFLAGS)	\
		$(SOUP_CFLAGS)		\
		$(WBXML_CFLAGS)		\
		$(LIBICAL_CFLAGS)	\
		$(LIBEBOOK_CFLAGS)	\
		$(GNOMEKEYRING_CFLAGS)	\
		-DG_LOG_DOMAIN=\"libeas\" \
		-I$(top_srcdir)/libeasmail/src/ \
		-DASKPASS=\"@ASKPASS@\"

libeas_la_LDFLAGS =			\
		$(GLIB_LIBS)		\
		$(EDATASERVER_LIBS)	\
		$(DBUS_LIBS)		\
		$(DBUS_GLIB_LIBS)	\
		$(LIBXML_LIBS)		\
		$(SOUP_LIBS)		\
		$(WBXML_LIBS)		\
		$(LIBICAL_LIBS)     \
		$(LIBEBOOK_LIBS)     \
		$(GNOMEKEYRING_LIBS)


libeas_la_LIBADD = ../../libeasaccount/src/libeasaccount.la


libeas_la_SOURCES =		\
        eas-connection.c \
        eas-connection.h \
        eas-connection-errors.h \
        eas-msg-base.h \
        eas-msg-base.c \
        eas-sync-folder-msg.h \
        eas-sync-folder-msg.c \
        eas-provision-msg.h \
        eas-provision-msg.c \
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
        eas-update-item-req.h \
        eas-update-item-req.c \
        eas-utils.h \
        eas-utils.c \
        eas-add-item-req.h \
        eas-add-item-req.c \
        eas-connection-errors.c \
        eas-move-email-req.h \
        eas-move-email-req.c \
        eas-move-email-msg.h \
        eas-move-email-msg.c \
        serialise_utils.c \
        serialise_utils.h\
        eas-ping-msg.h \
        eas-ping-msg.c \
        eas-ping-req.h \
        eas-ping-req.c \
        eas-con-info-translator.h \
        eas-con-info-translator.c


