EVOLUTION_privlib_LTLIBRARIES = libevoeas.la

libevoeas_la_CPPFLAGS = \
	$(CAMEL_CFLAGS)	\
	$(NULL)

libevoeas_la_SOURCES =					\
	camel-eas-settings.c				\
	camel-eas-settings.h				\
	$(NULL)

libevoeas_la_LIBADD =					\
	$(CAMEL_LIBS)					\
	$(NULL)

libevoeas_la_LDFLAGS = $(NO_UNDEFINED)

-include $(top_srcdir)/git.mk
