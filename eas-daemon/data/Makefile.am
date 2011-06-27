## Process this file with automake to produce Makefile.in
# Dbus service file
servicedir = $(dbus_servicesdir)
service_in_files = org.meego.activesyncd.service.in

service_DATA = $(service_in_files:.service.in=.service)

# Rule to make the service file with bindir expanded
$(service_DATA): $(service_in_files) Makefile
	@sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

#	@sed -e "s|\@bindir\@|$(bindir)|" $ $@

CLEANFILES = $(service_DATA)
EXTRA_DIST = $(service_in_files)
