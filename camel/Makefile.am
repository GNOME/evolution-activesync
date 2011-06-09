camel_provider_LTLIBRARIES = libcameleas.la
camel_provider_DATA = libcameleas.urls

libcameleas_la_CPPFLAGS = \
	$(AM_CPPFLAGS)				\
	-I$(top_srcdir)/src			\
	-I$(top_srcdir)/src/camel		\
	-I$(top_srcdir)/src/server	        \
	-I$(top_srcdir)/src/utils	        \
	$(CAMEL_CFLAGS)				\
	$(LIBEDATASERVER_CFLAGS)		\
	$(LIBECAL_CFLAGS)			\
	$(EVOLUTION_PLUGIN_CFLAGS)		\
	$(GNOME_INCLUDEDIR)			\
	$(SOUP_CFLAGS)                          \
	$(LIBEBACKEND_CFLAGS)			\
	$(E_DATA_SERVER_CFLAGS)			\
	-DG_LOG_DOMAIN=\"camel-eas-provider\"

libcameleas_la_SOURCES = 			\
	camel-eas-folder.c                	\
	camel-eas-store-summary.c              	\
	camel-eas-store.c                 	\
	camel-eas-summary.c               	\
	camel-eas-provider.c			\
	camel-eas-transport.c     	       	

noinst_HEADERS =         			\
	camel-eas-folder.h			\
	camel-eas-private.h			\
	camel-eas-store-summary.h              	\
	camel-eas-store.h			\
	camel-eas-summary.h			\
	camel-eas-transport.h		

libcameleas_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)

libcameleas_la_LIBADD =					\
	$(CAMEL_LIBS)                                   \
	$(EVOLUTION_PLUGIN_LIBS)			\
	$(LIBEDATASERVER_LIBS)				\
	$(LIBEBACKEND_LIBS)				\
	$(LIBECAL_LIBS)                                 \
	$(SOUP_LIBS)					\
	$(E_DATA_SERVER_LIBS)

EXTRA_DIST = libcameleas.urls

-include $(top_srcdir)/git.mk