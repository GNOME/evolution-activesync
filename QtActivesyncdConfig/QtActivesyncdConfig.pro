#-------------------------------------------------
#
# Project created by QtCreator 2011-08-17T09:51:51
#
#-------------------------------------------------

# This project file is not used in production builds; it is
# here to make it easy to use QtCreator in development. Please
# ensure that any changes here are also reflected (and tested)
# in Makefile.am.

# The Makefile in this directory can be generated either from
# Makefile.am (the normal method, achieved by running 'configure'
# in the parent directory), or by running 'qmake' in this directory
# to generate it from this .pro file.

QT       += core gui

# Prevent the 'signals' and 'slots' pseudo-keywords from being defined (as they clash
# with variable names in the C code). Now have to use Q_SLOTS and Q_SIGNALS instead.
CONFIG   += no_keywords

TARGET = QtActivesyncdConfig
TEMPLATE = app
CONFIG += link_pkgconfig

SOURCES += main.cpp\
        ConfigWizard.cpp

HEADERS  += ConfigWizard.h

FORMS    += ConfigWizard.ui


# Remember to add any new ones to Makefile.am too
PKGCONFIG += glib-2.0 libxml-2.0 libedataserver-1.2 gconf-2.0

# Remember to add any new libraries to Makefile.am too
LIBS += -L../eas-daemon/libeas -leas -Wl,-rpath -Wl,$(PWD)/../eas-daemon/libeas/.libs
LIBS += -L../libeasmail/src/.libs -leasmail -Wl,-rpath -Wl,$(PWD)/../libeasmail/src/.libs

OTHER_FILES += \
    readme.txt \
    Makefile.am
