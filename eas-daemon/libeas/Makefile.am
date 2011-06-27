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
		$(GNOMEKEYRING_LIBS)



libeas_la_SOURCES =		\
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
        
        


gconf_root = /apps/activesyncd/accounts

#account uids (identified using email)
email01 =andy@mobica.com
email02 =brahim@mobica.com
email03 =lorna@mobica.com
email04 =tez@mobica.com
email05 =jem@mobica.com
email05 =good.user@cstylianou.com

setaccounts: clearkeys
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email01)/serverUri https://cstylianou.com/Microsoft-Server-ActiveSync
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email01)/username andy
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email01)/password M0bica!
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email02)/serverUri https://cstylianou.com/Microsoft-Server-ActiveSync
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email02)/username brahim
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email02)/password M0bica!
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email03)/serverUri https://cstylianou.com/Microsoft-Server-ActiveSync
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email03)/username lorna
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email03)/password M0bica!
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email04)/serverUri https://cstylianou.com/Microsoft-Server-ActiveSync
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email04)/username tez
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email04)/password M0bica!
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email05)/serverUri https://cstylianou.com/Microsoft-Server-ActiveSync
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email05)/username good.user
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email05)/password G00dP@55w0rd


#use this option if you need to create a new GConf account
my_email= myname@mycompany.com
set-my-account:
	gconftool-2 --set --type string \
	            $(gconf_root)/$(my_email)/username MyUsername
	gconftool-2 --set --type string \
	            $(gconf_root)/$(my_email)/password MyPassword
	gconftool-2 --set --type string \
	            $(gconf_root)/$(my_email)/serverUri https://cstylianou.com/Microsoft-Server-ActiveSync

clear-my-account:
	@gconftool-2 --recursive-unset $(gconf_root)/$(my_email)

EXTRA_DIST = \
	eas-connection-errors.h


# Remove all application keys
clearkeys:
	@gconftool-2 --recursive-unset $(gconf_root)
