#include <check.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <stdlib.h>
#include <string.h>

#include "../eas-daemon/src/activesyncd-common-defs.h"
#include "../eas-daemon/tests/activesyncd-mail-client-stub.h"


GMainLoop* mainloop = NULL;
  
static void start_sync_completed(DBusGProxy* proxy, DBusGProxyCall* call, gpointer userData) {
  GError* error = NULL;
  gboolean rtn = TRUE;

	// check to see if any errors have been reported
  rtn = dbus_g_proxy_end_call(proxy,
                             call,
                             &error,
                             G_TYPE_INVALID);

  mark_point();
  if(error){
    fail_if(rtn == FALSE, "%s",&(*error->message));    
  }
  mark_point();  
  g_main_loop_quit (mainloop);  
}


START_TEST (test_daemon_connection)
{
  DBusGConnection* bus = NULL;
  DBusGProxy* remoteEasMail = NULL;
  GError* error = NULL;

	// initise G type library to allow use of G type objects which provide
	// a higher level of programming syntax including the psuedo object 
	// oriented approach supported by G types
  g_type_init();

	// initialisation of event loop within the program as required by any
	// program using G types
  mainloop = g_main_loop_new(NULL, FALSE);
  fail_if(mainloop == NULL,
    "Error: Failed to create the mainloop");  

  mark_point();
  // get an interface to the standard DBus messaging daemon
  bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
  mark_point();
  fail_if(bus == NULL,&(error->message));  

  mark_point();
  // create object to the EasMail object that will be exposed over the DBus 
  // interface and return a pointer to this object on the activesyncd 
  remoteEasMail = dbus_g_proxy_new_for_name(bus,
                              EAS_SERVICE_NAME,
                              EAS_SERVICE_MAIL_OBJECT_PATH /*EAS_SERVICE_OBJECT_PATH*/,
                              EAS_SERVICE_MAIL_INTERFACE   /*EAS_SERVICE_INTERFACE*/);
  fail_if(mainloop == NULL,
    "Error: Couldn't create the proxy object");  

  mark_point();
	// give any value here.  in this case -99 is the "known value" for this test
	// this value is not checked as part of the test, it is simply required that
	// something is passed from the client to the daemon
  gint testValue = -99;
  // generic dbus call to the "start_sync" method exposed by the EasMail object 
  // exposed by the daemon via DBus
  dbus_g_proxy_begin_call(remoteEasMail,  // name of interface object
                          "start_sync",   //  name of method on interface object
                          start_sync_completed,  // name of callback function that will be called when "start_sync" completes
                          NULL,  // ?
                          NULL,  // ?
                          G_TYPE_INT,  // type of test value
                          testValue,  // value passed to the start_sync method
                          G_TYPE_INVALID);  // ?

  mark_point();
	// start the mainloop
  g_main_loop_run(mainloop);
}
END_TEST

Suite* eas_daemon_suite(void)
{
  Suite* s = suite_create ("eas-daemon");

  /* daemon test case */
  TCase *tc_deamon = tcase_create ("core");
  suite_add_tcase (s, tc_deamon);
  tcase_add_test (tc_deamon, test_daemon_connection);


  return s;
}




