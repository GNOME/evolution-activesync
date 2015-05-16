# module-eas-mail-config is for evolution

evo_module_LTLIBRARIES = module-eas-mail-config.la

module_eas_mail_config_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir) \
	$(EVOLUTION_MAIL_CFLAGS) \
	$(LIBEDATASERVER_CFLAGS) \
	$(LIBEBACKEND_CFLAGS) \
	$(GCONF_CFLAGS) \
	$(NULL)

module_eas_mail_config_la_SOURCES = \
	module-eas-mail-config.c \
	e-mail-config-eas-backend.c \
	e-mail-config-eas-backend.h \
	$(NULL)

module_eas_mail_config_la_LIBADD = \
	$(EVOLUTION_MAIL_LIBS) \
	$(LIBEDATASERVER_LIBS) \
	$(LIBEBACKEND_LIBS) \
	$(GCONF_LIBS) \
	$(NULL)

module_eas_mail_config_la_LDFLAGS = \
	-module -avoid-version $(NO_UNDEFINED)
	$(NULL)
