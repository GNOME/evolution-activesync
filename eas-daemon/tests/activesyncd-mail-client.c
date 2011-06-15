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
 * Callback function
 */
static void start_sync_completed (DBusGProxy* proxy, DBusGProxyCall* call, gpointer userData)
{
    GError* error = NULL;

    g_debug ("start_sync Completed");

    // check to see if any errors have been reported
    if (!dbus_g_proxy_end_call (proxy,
                                call,
                                &error,
                                G_TYPE_INVALID))
    {
        g_error (" Error: %s", error->message);
        g_error_free (error);
    }
    else
    {
        g_debug ("start_sync Success ");
    }
}


/**
 *
 */
int main (int argc, char** argv)
{
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
    mainloop = g_main_loop_new (NULL, FALSE);
    if (mainloop == NULL)
    {
        g_error ("Error: Failed to create the mainloop");
        exit (EXIT_FAILURE);
    }


    g_debug ("Connecting to Session D-Bus.");
    // get an interface to the standard DBus messaging daemon
    bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
    if (bus == NULL)
    {
        g_error ("Error: Couldn't connect to the Session bus (%s) ", error->message);
        exit (EXIT_FAILURE);
    }

    g_debug ("Creating a GLib proxy object for EasMail.");
    // create object to the EasMail object that will be exposed over the DBus
    // interface and return a pointer to this object on the activesyncd
    remoteEasMail = dbus_g_proxy_new_for_name (bus,
                                               EAS_SERVICE_NAME,
                                               EAS_SERVICE_MAIL_OBJECT_PATH /*EAS_SERVICE_OBJECT_PATH*/,
                                               EAS_SERVICE_MAIL_INTERFACE   /*EAS_SERVICE_INTERFACE*/);
    if (remoteEasMail == NULL)
    {
        g_error ("Error: Couldn't create the proxy object");
        exit (EXIT_FAILURE);
    }

    // give any value here.  in this case -99 is the "known value" for this test
    // this value is not checked as part of the test, it is simply required that
    // something is passed from the client to the daemon
    gint testValue = -99;
    // generic dbus call to the "start_sync" method exposed by the EasMail object
    // exposed by the daemon via DBus
    dbus_g_proxy_begin_call (remoteEasMail, // name of interface object
                             "start_sync",   //  name of method on interface object
                             start_sync_completed,  // name of callback function that will be called when "start_sync" completes
                             NULL,  // ?
                             NULL,  // ?
                             G_TYPE_INT,  // type of test value
                             testValue,  // value passed to the start_sync method
                             G_TYPE_INVALID);  // ?

    g_debug ("start_sync launched");

    // start the mainloop
    g_main_loop_run (mainloop);

    return 0;  // return program exit success
}

