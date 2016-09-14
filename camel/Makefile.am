camel_provider_LTLIBRARIES = libcameleas.la
camel_provider_DATA = libcameleas.urls

libcameleas_la_CFLAGS = $(WARNING_FLAGS)

libcameleas_la_CPPFLAGS = \
	$(AM_CPPFLAGS)				\
	$(CAMEL_CFLAGS)				\
	$(LIBEDATASERVER_CFLAGS)		\
	$(LIBECAL_CFLAGS)			\
	$(GNOME_INCLUDEDIR)			\
	$(SOUP_CFLAGS)                          \
	$(LIBEBACKEND_CFLAGS)			\
	$(E_DATA_SERVER_CFLAGS)			\
	$(DBUS_GLIB_CFLAGS)			\
	-DG_LOG_DOMAIN=\"camel-eas-provider\"	\
	-I$(top_srcdir)/camel			\
	-I$(top_srcdir)/libeasclient		\
	-I$(top_srcdir)/libevoeas		\
	$(EVOLUTION_MAIL_CFLAGS)		\
	$(NULL)

libcameleas_la_SOURCES = 			\
	camel-eas-folder.c                	\
	camel-eas-message-info.c               	\
	camel-eas-store-summary.c              	\
	camel-eas-store.c                 	\
	camel-eas-summary.c               	\
	camel-eas-provider.c			\
	camel-eas-utils.c			\
	camel-eas-transport.c			\
	$(NULL)

noinst_HEADERS =         			\
	camel-eas-folder.h			\
	camel-eas-message-info.h		\
	camel-eas-private.h			\
	camel-eas-store-summary.h              	\
	camel-eas-store.h			\
	camel-eas-summary.h			\
	camel-eas-utils.h			\
	camel-eas-transport.h			\
	$(NULL)


libcameleas_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)

libcameleas_la_LIBADD =					\
	$(top_builddir)/libeasclient/libeasclient.la	\
	$(top_builddir)/libevoeas/libevoeas.la		\
	$(CAMEL_LIBS)                                   \
	$(LIBEDATASERVER_LIBS)				\
	$(LIBEBACKEND_LIBS)				\
	$(LIBECAL_LIBS)                                 \
	$(SOUP_LIBS)					\
	$(DBUS_GLIB_LIBS)				\
	$(E_DATA_SERVER_LIBS)				\
	$(EVOLUTION_MAIL_LIBS)				\
	$(NULL)

EXTRA_DIST = libcameleas.urls

-include $(top_srcdir)/git.mk
