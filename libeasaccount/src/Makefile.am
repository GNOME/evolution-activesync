## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES = libeasaccount.la

AM_CXXFLAGS = $(INTI_CXXFLAGS) $(WARNING_FLAGS)

libeasaccount_la_CFLAGS = $(WARNING_FLAGS)

libeasaccount_la_CPPFLAGS =			\
		$(GLIB_CFLAGS)		\
		$(GIO_CFLAGS) \
		$(EDATASERVER_CFLAGS) \
		$(NULL)

libeasaccount_la_LDFLAGS =			\
		$(GLIB_LIBS)		\
		$(GIO_LIBS) \
		$(EDATASERVER_LIBS) \
		$(NULL)


libeasaccount_la_SOURCES =	eas-account.h eas-account.c \
				eas-account-list.h eas-account-list.c \
				$(NULL)


libeasaccountincludedir = $(privincludedir)/eas-account
libeasaccountinclude_HEADERS = eas-account.h eas-account-list.h

gconf_root = /apps/activesyncd/accounts

#account uids (identified using email)
email01 =andy@mobica.com
email02 =brahim@cstylianou.com
email03 =lorna@mobica.com
email04 =tez@mobica.com
email05 =good.user@cstylianou.com
email06 =jem@mobica.com
email07 =Salvatore@cstylianou.com

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
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email06)/serverUri https://cstylianou.com/Microsoft-Server-ActiveSync
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email06)/username jem
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email06)/password M0bica!
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email07)/serverUri https://cstylianou.com/Microsoft-Server-ActiveSync
	gconftool-2 --set --type string \
				$(gconf_root)/$(email07)/username Salvatore
	gconftool-2 --set --type string \
	            $(gconf_root)/$(email07)/password M0bica!

#use this option if you need to create a new GConf account
my_email= myname@mycompany.com
set-my-account:
	gconftool-2 --set --type string \
	            $(gconf_root)/$(my_email)/username MyUsername
	gconftool-2 --set --type string \
	            $(gconf_root)/$(my_email)/serverUri https://cstylianou.com/Microsoft-Server-ActiveSync
	gconftool-2 --set --type string \
	            $(gconf_root)/$(my_email)/password M0bica!

clear-my-account:
	@gconftool-2 --recursive-unset $(gconf_root)/$(my_email)


# Remove all application keys
clearkeys:
	@gconftool-2 --recursive-unset $(gconf_root)
