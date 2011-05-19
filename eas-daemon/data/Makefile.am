## Process this file with automake to produce Makefile.in
# Dbus service file
servicedir = $(DBUS_SERVICES_DIR)
service_in_files = org.meego.activesyncd.service.in

service_DATA = $(service_in_files:.service.in=.service)

# Rule to make the service file with bindir expanded
$(service_DATA): $(service_in_files) Makefile
	@sed -e "s|@bindir@|$(bindir)|" $<> $@

#	@sed -e "s|\@bindir\@|$(bindir)|" $ $@

CLEANFILES = $(service_DATA)
