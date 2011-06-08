TARGET = eas-autodiscover-test
CC = gcc
OBJECTS = eas-autodiscover-test.o

EDATASERVER_CFLAGS = -DORBIT2=1 -pthread -I/usr/include/evolution-data-server-2.28 -I/usr/include/libxml2 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/gconf/2 -I/usr/include/libsoup-2.4 -I/usr/include/libbonobo-2.0 -I/usr/include/orbit-2.0 -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -I/usr/include/bonobo-activation-2.0  

EDATASERVER_LIBS = -pthread -ledataserver-1.2 -lxml2 -lgconf-2 -lsoup-2.4 -lbonobo-2 -lgio-2.0 -lbonobo-activation -lgmodule-2.0 -lORBit-2 -lgobject-2.0 -lgthread-2.0 -lrt -lglib-2.0  

CFLAGS = -g -Wall $(EDATASERVER_CFLAGS)

LIBS = -L/lib -lrt -lwbxml2 $(EDATASERVER_LIBS)
STATIC_LIBS = ../eas-daemon/libeas/libeas.la ../libeascal/src/libeascal.la ../libeasmail/src/libeasmail.la

eas-autodiscover-test : $(OBJECTS)
	libtool --mode=link $(CC) $(CFLAGS) $(OBJECTS) $(LIBS) $(STATIC_LIBS) -o $(TARGET)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean: 
	rm *.o eas-autodiscover-test

