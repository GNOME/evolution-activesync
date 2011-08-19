include $(top_srcdir)/m4-repo/autotroll.mk

bin_PROGRAMS=QtActivesyncdConfig


QtActivesyncdConfig_SOURCES = main.cpp ConfigWizard.cpp ConfigWizard.h ConfigWizard.moc.cpp ConfigWizard.ui.h

QtActivesyncdConfig_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS) $(GLIB_CFLAGS) $(LIBXML_CFLAGS)
QtActivesyncdConfig_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS)
QtActivesyncdConfig_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS)
QtActivesyncdConfig_LDADD    = $(QT_LIBS) $(LDADD) ../libeasaccount/src/libeasaccount.la

# NO NO NO NO NO! Nothing but the dæmon may use libeas directly!
QtActivesyncdConfig_LDADD   += $(QT_LIBS) $(LDADD) ../eas-daemon/libeas/libeas.la


BUILT_SOURCES = ConfigWizard.moc.cpp ConfigWizard.ui.h
