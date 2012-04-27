### EVO_PLUGIN_RULE ### begin ###

%.eplug: %.eplug.in
	sed -e 's|\@PLUGINDIR\@|$(plugindir)|'		\
	-e 's|\@SOEXT\@|$(SOEXT)|'			\
	-e 's|\@GETTEXT_PACKAGE\@|$(GETTEXT_PACKAGE)|'	\
	-e 's|\@LOCALEDIR\@|$(localedir)|' $< > $@

%.eplug.in: %.eplug.xml
	LC_ALL=C $(INTLTOOL_MERGE) -x -u /tmp $< $@

%.error: %.error.xml
	LC_ALL=C $(INTLTOOL_MERGE) -x -u /tmp $< $@

### EVO_PLUGIN_RULE ### end ###


# plugin_DATA = org-gnome-exchange-activesync.eplug
# plugin_LTLIBRARIES = liborg-gnome-exchange-activesync.la
noinst_LTLIBRARIES = liborg-gnome-exchange-activesync.la

liborg_gnome_exchange_activesync_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(EVOLUTION_DATA_SERVER_CFLAGS) \
	$(EVOLUTION_PLUGIN_CFLAGS) \
	$(EVOLUTION_SHELL_CFLAGS) \
	$(EVOLUTION_MAIL_CFLAGS) \
	$(LDAP_CFLAGS)		\
	$(SYNCEVOLUTION_DBUS_CFLAGS) \
	-I$(top_srcdir)/libeasaccount/src -I$(top_srcdir)/libeasclient -I$(top_srcdir)/camel


liborg_gnome_exchange_activesync_la_SOURCES = 	\
	activesync-account-setup.c	\
	eas-account-listener.c	\
	eas-account-listener.h

liborg_gnome_exchange_activesync_la_LIBADD = \
	$(top_builddir)/libeasclient/libeasclient.la \
	$(EVOLUTION_DATA_SERVER_LIBS) \
	$(EVOLUTION_PLUGIN_LIBS) \
	$(EVOLUTION_SHELL_LIBS) \
	$(EVOLUTION_MAIL_LIBS) \
	$(LDAP_LIBS) \
	$(SYNCEVOLUTION_DBUS_LIBS) \
	$(DNS_LIBS)

liborg_gnome_exchange_activesync_la_LDFLAGS = \
	-module -avoid-version $(NO_UNDEFINED)

error_DATA = org-gnome-exchange-activesync.error
errordir = $(EVOLUTION_PLUGIN_errordir)

EXTRA_DIST = \
	org-gnome-exchange-activesync.eplug.xml \
	org-gnome-exchange-activesync.error.xml

BUILT_SOURCES = $(error_DATA) org-gnome-exchange-activesync.eplug 

CLEANFILES = $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk

