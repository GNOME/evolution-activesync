## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES = libeas.la

AM_CXXFLAGS = $(INTI_CXXFLAGS)

INCLUDES = -I../../libeasmail/src
	  

libeas_la_CPPFLAGS =			\
		$(GLIB_CFLAGS)		\
		$(EDATASERVER_CFLAGS)	\
		$(DBUS_CFLAGS)		\
		$(DBUS_GLIB_CFLAGS)	\
		$(LIBXML_CFLAGS)	\
		$(SOUP_CFLAGS)		\
		$(WBXML_CFLAGS)		\
		$(LIBICAL_CFLAGS)	\
		$(GNOMEKEYRING_CFLAGS)	\
		-DG_LOG_DOMAIN=\"libeas\" \
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
		$(GNOMEKEYRING_LIBS)



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
        eas-connection-errors.c \
        eas-account.h \
        eas-account.c \
        eas-account-list.h \
        eas-account-list.c \
        eas-move-email-req.h \
        eas-move-email-req.c \
        eas-move-email-msg.h \
        eas-move-email-msg.c \
        serialise_utils.c \
        serialise_utils.h\
        eas-ping-msg.h \
        eas-ping-msg.c \
        eas-ping-req.h \
        eas-ping-req.c
        
        


gconf_root = /apps/activesyncd

DISTCLEANFILES = $(schema_DATA)

#schemadir=$(GCONF_SCHEMA_FILE_DIR)
#schema_in_files=activesyncd-accounts.schemas.in
#schema_DATA=$(schema_in_files:.schemas.in=.schemas)


if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(schemas_DATA)
endif 


#if GCONF_SCHEMAS_INSTALL
#install-data-local:
#	if test -z "$(DESTDIR)" ; then							\
#		for p in $(schema_DATA) ; do						\
#			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) 		\
#				$(GCONFTOOL) --makefile-install-rule $$p;		\
#		done									\
#	fi
#endif

accountinfo = \
"[<?xml version=\"1.0\"?>\
	<account uid=\"tez@cstylianou.com\">\
		<uri>https://cstylianou.com/Microsoft-Server-ActiveSync</uri>\
		<username>tez</username>\
		<password>M0bica!</password>\
	</account>,<?xml version=\"1.0\"?>\
	<account uid=\"1303900871.4417.1@brahim-laptop\">\
		<uri>https://cstylianou.com/Microsoft-Server-ActiveSync</uri>\
		<username>brahim</username>\
		<password>M0bica!</password>\
	</account>,<?xml version=\"1.0\"?>\
	<account uid=\"1303900871.4418.1@lorna-laptop\">\
		<uri>https://cstylianou.com/Microsoft-Server-ActiveSync</uri>\
		<username>lorna</username>\
		<password>M0bica!</password>\
	</account>,<?xml version=\"1.0\"?>\
	<account uid=\"123456789@andygould\">\
		<uri>https://cstylianou.com/Microsoft-Server-ActiveSync</uri>\
		<username>andy</username>\
		<password>M0bica!</password>\
	</account>,<?xml version=\"1.0\"?>\
	<account uid=\"good.user@cstylianou.com\">\
		<uri>https://cstylianou.com/Microsoft-Server-ActiveSync</uri>\
		<username>good.user</username>\
		<password>G00dP@55w0rd</password>\
	</account>\
	]"

setaccount: clearkeys
	gconftool-2 --set --type list --list-type=string\
		$(gconf_root)/accounts $(accountinfo)

# Remove all application keys
clearkeys:
	@gconftool-2 --recursive-unset $(gconf_root)
