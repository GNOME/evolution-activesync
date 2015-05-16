eds_module_LTLIBRARIES = module-eas-backend.la

module_eas_backend_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir) \
	$(EVOLUTION_MAIL_CFLAGS) \
	$(LIBEDATASERVER_CFLAGS) \
	$(LIBEBACKEND_CFLAGS) \
	$(NULL)

module_eas_backend_la_SOURCES = \
	module-eas-backend.c \
	e-eas-backend.c \
	e-eas-backend.h \
	e-eas-backend-factory.c \
	e-eas-backend-factory.h \
	$(NULL)

module_eas_backend_la_LIBADD = \
	$(EVOLUTION_MAIL_LIBS) \
	$(LIBEDATASERVER_LIBS) \
	$(LIBEBACKEND_LIBS) \
	$(NULL)

module_eas_backend_la_LDFLAGS = \
	-module -avoid-version $(NO_UNDEFINED) \
	$(NULL)
