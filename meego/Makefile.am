# This is a "proxy" Makefile that faciliates integration of the qmake
# based ActiveSync settings UI build system.

UIDIR = meego-ux-settings-activesync

all-local:
	cd $(UIDIR) && $(MAKE) $(AM_MAKEFLAGS) all

clean-local:
	cd $(UIDIR) && $(MAKE) $(AM_MAKEFLAGS) clean

distclean-local:
	cd $(UIDIR) && $(MAKE) $(AM_MAKEFLAGS) distclean

install:
	cd $(UIDIR) && $(MAKE) $(AM_MAKEFLAGS) install

uninstall-local:
	cd $(UIDIR) && $(MAKE) $(AM_MAKEFLAGS) uninstall

EXTRA_DIST = \
	$(UIDIR)/*.[hc]pp \
	$(UIDIR)/*qml* \
	$(UIDIR)/*.desktop \
	$(UIDIR)/*.pro \
	$(UIDIR)/README \
	$(UIDIR)/TODO \
	$(NULL)
