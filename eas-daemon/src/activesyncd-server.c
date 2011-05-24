/**
 *  
 *  Filename: activesyncd-server.c
 *  Project: activesyncd
 *  Description: Daemon setup, DBus setup, GObjects construction and registration.
 *
 */

//system include
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <libsoup/soup.h>


//user include
#include "activesyncd-common-defs.h"
#include "../libeas/eas-connection.h"
#include "../libeas/eas-accounts.h"
#include "eas-calendar.h" 
#include "eas-common.h"
#include "eas-contact.h" 
#include "eas-mail.h"

#ifdef DISABLE_EAS_DAEMON
#define dbg(fmtstr, args...) (g_debug(":%s: " fmtstr "", __func__, ##args))
#else
#define dbg(dummy...)
#endif


/*
  activesyncd entry point
*/
int main(int argc, char** argv) {
    DBusGConnection* bus = NULL;
    DBusGProxy* busProxy = NULL;
    EasAccounts* EasAccounts = NULL;
    EasConnection* EasConnObj = NULL;
    EasCalendar* EasCalendarObj = NULL;
    EasCommon* EasCommonObj = NULL;
    EasContact* EasContactObj = NULL;
    EasMail*EasMailObj = NULL;
    
    GMainLoop* mainloop = NULL;
    guint result;
    GError* error = NULL;


    g_type_init();

    mainloop = g_main_loop_new(NULL, FALSE);
    if (mainloop == NULL) {
        g_debug("Error: Couldn't create GMainLoop");
        exit(EXIT_FAILURE);
    }

    //Creating all the GObjects
    g_debug("activesyncd Daemon Started");

   g_debug("creating acounts object\n");   
   EasAccounts = eas_accounts_new ();
   
   g_debug("eas_accounts_read_accounts_info\n");    
    int err = eas_accounts_read_accounts_info(EasAccounts);
    if (err !=0)
    {
        g_debug("Error reading data from file accounts.cfg\n");
        g_main_loop_quit (mainloop);
        exit(err);    
    }

   g_debug("getting data from EasAccounts object\n"); 
   
    //TODO:  handling mltiple connections (connections per account)
    guint64 accountId;
   accountId =1234567890;
   
   gchar* serverUri = NULL;
   gchar* username = NULL;
   gchar* password = NULL;

    //TODO:  handling mltiple connections (connections per account)
   serverUri = eas_accounts_get_server_uri (EasAccounts, accountId);
   username = eas_accounts_get_user_id (EasAccounts, accountId);
   password = eas_accounts_get_password (EasAccounts, accountId);

    g_debug("Creating EEasConnection GObject.\n");
    GError* cnc_error = NULL;

    //TODO:   EasConnection -no need to pass this params, they are read form config file
    EasConnObj = eas_connection_new(serverUri, username, password, &cnc_error);
    if (EasConnObj == NULL) {
        g_debug("Error: Failed to create EEasConnection instance");
        g_clear_error (&error);
        g_main_loop_quit (mainloop);
        exit(EXIT_FAILURE);
    }

    g_debug("Creating calendar  gobject.");
    EasCalendarObj = g_object_new(EAS_TYPE_CALENDAR , NULL);
    if (EasCalendarObj == NULL) {
        g_debug("Error: Failed to create calendar  instance");
        g_main_loop_quit (mainloop);
        exit(EXIT_FAILURE);
    }

    g_debug("Creating common  gobject.");
    EasCommonObj = g_object_new(EAS_TYPE_COMMON , NULL);
    if (EasCommonObj == NULL) {
        g_debug("Error: Failed to create common  instance");
        g_main_loop_quit (mainloop);
        exit(EXIT_FAILURE);
    }

    g_debug("Creating contact  gobject.");
    EasContactObj = g_object_new(EAS_TYPE_CONTACT , NULL);
    if (EasContactObj == NULL) {
        g_debug("Error: Failed to create common  instance");
        g_main_loop_quit (mainloop);
        exit(EXIT_FAILURE);
    }

    g_debug("Creating mail  gobject.");
    EasMailObj = eas_mail_new ();
    if (EasMailObj == NULL) {
        g_debug("Error: Failed to create common  instance");
        g_main_loop_quit (mainloop);
        exit(EXIT_FAILURE);
    }

    g_debug("Pass a EasConnection handle to the exposed GObjects");
    //ret = eas_calendar_set_eas_connection(EasCalendarObj, EasConnObj);
    //ret = eas_common_set_eas_connection(EasCommonObj, EasConnObj);
    //ret = eas_contact_set_eas_connection(EasContactObj, EasConnObj);
   eas_mail_set_eas_connection(EasMailObj, EasConnObj);
   
    g_debug("Connecting to the session DBus");
    bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if (error != NULL) {
        g_debug("Error: Connecting to the session DBus (%s)", error->message);
        g_clear_error (&error);
        g_main_loop_quit (mainloop);
        exit(EXIT_FAILURE);
    }

    g_debug("Registering the well-known name (%s)", EAS_SERVICE_NAME);
    busProxy = dbus_g_proxy_new_for_name(bus,
                                       DBUS_SERVICE_DBUS,
                                       DBUS_PATH_DBUS,
                                       DBUS_INTERFACE_DBUS);
    if (busProxy == NULL) {
        g_debug("Error: Failed to get a proxy for D-Bus");
        g_main_loop_quit (mainloop);
        exit(EXIT_FAILURE);
  }

  /* register the well-known name.*/
    g_debug("D-Bus RequestName RPC ");
    if (!dbus_g_proxy_call(busProxy,
                         "RequestName",
                         &error,
                         G_TYPE_STRING,
                         EAS_SERVICE_NAME,
                         G_TYPE_UINT,
                         0,
                         G_TYPE_INVALID,
                         G_TYPE_UINT,
                         &result,
                         G_TYPE_INVALID)) {
   g_debug("Error: D-Bus RequestName RPC failed (%s)", error->message);
   g_main_loop_quit (mainloop);
   exit(EXIT_FAILURE);
  }

    g_debug("RequestName returned %d", result);
    if (result != 1) {
        g_debug("Error: Failed to get the primary well-known name");
       exit(EXIT_FAILURE);
  }


/*
 //we don't want EasConnObj to be exposed on DBus Interface anymore
    g_debug("Registering Gobjects on the D-Bus.");
    dbus_g_connection_register_g_object(bus,
                                      EAS_SERVICE_OBJECT_PATH,
                                      G_OBJECT(EasConnObj));

*/

    //	Registering  calendar Gobject 
    dbus_g_connection_register_g_object(bus,
                                      EAS_SERVICE_CALENDAR_OBJECT_PATH,
                                      G_OBJECT(EasCalendarObj));

    //	Registering  common Gobject
    dbus_g_connection_register_g_object(bus,
                                      EAS_SERVICE_COMMON_OBJECT_PATH,
                                      G_OBJECT(EasCommonObj));

    //	Registering  contact Gobject
    dbus_g_connection_register_g_object(bus,
                                      EAS_SERVICE_CONTACT_OBJECT_PATH,
                                      G_OBJECT(EasContactObj));

    //	Registering  mail Gobject
    dbus_g_connection_register_g_object(bus,
                                      EAS_SERVICE_MAIL_OBJECT_PATH,
                                      G_OBJECT(EasMailObj));


    g_debug("Ready to serve requests");

#ifndef DISABLE_EAS_DAEMON
    if (daemon(0, 0) != 0) {
        g_debug("Failed to daemonize");
  }
#else
    g_debug("Not daemonizing (built with DISABLE_EAS_DAEMON)");
#endif

    g_main_loop_run(mainloop);

   // Clean up
   g_main_loop_unref (mainloop);

    return 0;
}

