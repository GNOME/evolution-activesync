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

TARGET = QtActivesyncdConfig
TEMPLATE = app

LIBS += -L../libeasaccount/src/.libs -leasaccount -Wl,-rpath -Wl,$(PWD)/../libeasaccount/src/.libs

SOURCES += main.cpp\
        ConfigWizard.cpp

HEADERS  += ConfigWizard.h

FORMS    += ConfigWizard.ui

# Remember to add any new ones to Makefile.am too, and to do it *properly*
# there using the appropriate FOO_CFLAGS variables rather than hard-coded
# directories which are not portable.
INCLUDEPATH +=  /usr/include/glib-2.0 \
                /usr/lib/glib-2.0/include \
                /usr/include/libxml2 \
                /usr/include/evolution-data-server-2.32 \
                /usr/include/gconf/2

# Remember to add any new libraries to Makefile.am too
LIBS +=         -lglib-2.0 \
                -lxml2
LIBS +=         -L../eas-daemon/libeas -leas

OTHER_FILES += \
    readme.txt \
    Makefile.am
