## Process this file with automake to produce Makefile.in
bin_PROGRAMS = activesyncd-test-001
#bin_PROGRAMS = activesyncd_test_email_folder_hierarchy

interface_xml =  ../src/eas-mail.xml
commondefs = ../src/activesyncd-common-defs.h


INCLUDES = -I/usr/include/dbus-1.0 \
	  -I/usr/lib/dbus-1.0/include \
	  -I/usr/include/glib-2.0 \
	  -I/usr/lib/glib-2.0/include

LIBS =  -L/usr/lib \
	-ldbus-1 \
	-lpthread \
	-lrt \
	-lglib-2.0 \
	-ldbus-glib-1 \
	-lgobject-2.0 \
	-lgthread-2.0 

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

