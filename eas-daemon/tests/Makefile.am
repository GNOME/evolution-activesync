## Process this file with automake to produce Makefile.in
bin_PROGRAMS = activesyncd-test-001
#bin_PROGRAMS = activesyncd_test_email_folder_hierarchy

interface_xml =  ../src/eas-mail.xml
commondefs = ../src/activesyncd-common-defs.h


INCLUDES =				\
		$(GLIB_CFLAGS)		\
		$(EDATASERVER_CFLAGS)	\
		$(DBUS_CFLAGS)		\
		$(DBUS_GLIB_CFLAGS)

LIBS =					\
		$(GLIB_LIBS)		\
		$(EDATASERVER_LIBS)	\
		$(DBUS_LIBS)		\
		$(DBUS_GLIB_LIBS)


BUILT_SOURCES = activesyncd-mail-client-stub.h

#Rule to generate the binding headers
activesyncd-mail-client-stub.h: $(interface_xml)
	dbus-binding-tool --prefix=eas_mail --mode=glib-client $< > $@

AM_CFLAGS = $(INTI_CFLAGS)
activesyncd_test_001_SOURCES = activesyncd-mail-client-stub.h activesyncd-test-001.c
#activesyncd_mail_client_SOURCES = activesyncd-mail-client-stub.h activesyncd-test-001.c
#activesyncd_mail_client_SOURCES = activesyncd-mail-client-stub.h activesyncd-mail-client.c
#activesyncd_mail_client_SOURCES = activesyncd-mail-client-stub.h activesyncd_test_email_folder_hierarchy.c
activesyncd_test_001_LDADD = $(INTI_LIBS)

# Correctly clean the generated headers, but keep the xml description
CLEANFILES = $(BUILT_SOURCES)

