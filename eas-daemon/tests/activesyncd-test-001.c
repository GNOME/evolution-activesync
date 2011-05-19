/**
 *  
 *  Filename: activesyncd-mail-client.c
 *  Project: activesyncd 
 *  Description: Tests the activesyncd daemon interfaces using dbus-glib.
 *
 */

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <stdlib.h>
#include <string.h>

#include "../src/activesyncd-common-defs.h"
#include "activesyncd-mail-client-stub.h"




/**
 *
 */
int main(int argc, char** argv) {
  DBusGConnection* bus = NULL;
  DBusGProxy* remoteEasMail = NULL;
  GMainLoop* mainloop = NULL;
  GError* error = NULL;

	// initise G type library to allow use of G type objects which provide
	// a higher level of programming syntax including the psuedo object 
	// oriented approach supported by G types
  g_type_init();

	// initialisation of event loop within the program as required by any
	// program using G types
  mainloop = g_main_loop_new(NULL, FALSE);
  if (mainloop == NULL) {
    g_printerr("Error: Failed to create the mainloop");
    exit(EXIT_FAILURE);
  }
  

  g_print("Connecting to Session D-Bus.\n");
  // get an interface to the standard DBus messaging daemon
  bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
  if (bus == NULL) {
    g_printerr("Error: Couldn't connect to the Session bus (%s) \n", error->message);
    exit(EXIT_FAILURE);
  }

  g_print("Creating a GLib proxy object for EasMail.\n");
  // create object to the EasMail object that will be exposed over the DBus 
  // interface and return a pointer to this object on the activesyncd 
  remoteEasMail = dbus_g_proxy_new_for_name(bus,
                              EAS_SERVICE_NAME,
                              EAS_SERVICE_MAIL_OBJECT_PATH /*EAS_SERVICE_OBJECT_PATH*/,
                              EAS_SERVICE_MAIL_INTERFACE   /*EAS_SERVICE_INTERFACE*/);
  if (remoteEasMail == NULL) {
    g_printerr("Error: Couldn't create the proxy object\n");
    exit(EXIT_FAILURE);
  }


  gchar *sync_key = NULL;
  GError* err = NULL;
  gboolean ret = FALSE;

  g_print("making the call...\n");
  ret = dbus_g_proxy_call(remoteEasMail,  // name of interface object
                          "test_001",   //  name of method on interface object
			  &err,
			  G_TYPE_INVALID,
                          G_TYPE_STRING, &sync_key,
                          G_TYPE_INVALID);

 if(ret){
	//no error check the return string
	g_print("the srting return = %s\n", sync_key );
 }
else{
     //there is an error check the its message
     g_printerr(" Error: %s\n", err->message);
     g_main_loop_quit (mainloop);
  }


  // start the mainloop
  g_main_loop_run(mainloop);

  return 0;  // return program exit success
}

