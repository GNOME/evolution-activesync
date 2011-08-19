#-------------------------------------------------
#
# Project created by QtCreator 2011-08-17T09:51:51
#
#-------------------------------------------------

QT       += core gui

TARGET = QtActivesyncdConfig
TEMPLATE = app


SOURCES += main.cpp\
        ConfigWizard.cpp

HEADERS  += ConfigWizard.h

FORMS    += ConfigWizard.ui

INCLUDEPATH +=  /usr/include/glib-2.0 \
                /usr/lib/glib-2.0/include \
                /usr/include/libxml2

LIBS +=         -lglib-2.0 \
                -lxml2
LIBS +=         -L../eas-daemon/libeas -leas

OTHER_FILES += \
    readme.txt
