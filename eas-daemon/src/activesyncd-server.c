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
#include "eas-sync.h"
#include "eas-common.h"
#include "eas-mail.h"

#include "../../logger/eas-logger.h"

#ifdef DISABLE_EAS_DAEMON
#define dbg(fmtstr, args...) (g_debug(":%s: " fmtstr "", __func__, ##args))
#else
#define dbg(dummy...)
#endif
#if 0
static GLogFunc g_default_logger = NULL;

void eas_logger (const gchar *log_domain,
                 GLogLevelFlags log_level,
                 const gchar *message,
                 gpointer user_data)
{
    FILE *logfile = NULL;
    pid_t pid = getpid();
    int envLevel = 4;

    if (getenv ("EAS_DEBUG_FILE"))
    {
        logfile = fopen (g_getenv ("EAS_DEBUG_FILE"), "a");
    }

    if (getenv ("EAS_DEBUG"))
    {
        envLevel = atoi (g_getenv ("EAS_DEBUG"));
    }

    if (log_level == G_LOG_LEVEL_ERROR && g_default_logger)
    {
        g_default_logger (log_domain, log_level, message, user_data);
        return;
    }

    if (logfile)
    {
        fprintf (logfile, "(process:%d): ", pid);

        if (log_domain)
        {
            fprintf (logfile, "%s-", log_domain);
        }

        if (envLevel > 0 && log_level == G_LOG_LEVEL_CRITICAL)
            fprintf (logfile, "*** CRITICAL ***:%s \n", message);

        if (envLevel > 1 && log_level == G_LOG_LEVEL_WARNING)
            fprintf (logfile, "WARNING **:%s\n", message);

        if (envLevel > 2 && log_level == G_LOG_LEVEL_MESSAGE)
            fprintf (logfile, "Message:%s\n", message);

        if (envLevel > 3 && log_level == G_LOG_LEVEL_DEBUG)
            fprintf (logfile, "DEBUG:%s\n", message);

        fclose (logfile);
    }
    else
    {
        printf ("(process:%d): ", pid);

        if (log_domain)
        {
            printf ("%s-", log_domain);
        }

        if (envLevel > 0 && log_level == G_LOG_LEVEL_CRITICAL)
            printf ("*** CRITICAL ***:%s \n", message);

        if (envLevel > 1 && log_level == G_LOG_LEVEL_WARNING)
            printf ("WARNING **:%s\n", message);

        if (envLevel > 2 && log_level == G_LOG_LEVEL_MESSAGE)
            printf ("Message:%s\n", message);

        if (envLevel > 3 && log_level == G_LOG_LEVEL_DEBUG)
            printf ("DEBUG:%s\n", message);
    }
}
#endif

/*
  activesyncd entry point
*/
int main (int argc, char** argv)
{
    DBusGConnection* bus = NULL;
    DBusGProxy* busProxy = NULL;
//    EasConnection* EasConnObj = NULL;
    EasSync* EasSyncObj = NULL;
    EasCommon* EasCommonObj = NULL;
    EasMail*EasMailObj = NULL;

    GMainLoop* mainloop = NULL;
    guint result;
    GError* error = NULL;

    g_type_init();
#if 0
    g_log_set_handler (NULL,
                       G_LOG_LEVEL_DEBUG | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL,
                       eas_logger,
                       NULL);
#endif
    g_log_set_default_handler (eas_logger, NULL);

    mainloop = g_main_loop_new (NULL, FALSE);
    if (mainloop == NULL)
    {
        g_debug ("Error: Couldn't create GMainLoop");
        exit (EXIT_FAILURE);
    }

    //Creating all the GObjects
    g_debug ("activesyncd Daemon Started");

#if 0
    g_debug ("Creating EEasConnection GObject.");
    EasConnObj = eas_connection_new();
    if (EasConnObj == NULL)
    {
        g_debug ("Error: Failed to create EEasConnection instance");
        g_clear_error (&error);
        g_main_loop_quit (mainloop);
        exit (EXIT_FAILURE);
    }
#endif

    g_debug ("Creating calendar  gobject.");
    EasSyncObj = eas_sync_new();
    if (EasSyncObj == NULL)
    {
        g_debug ("Error: Failed to create calendar  instance");
        g_main_loop_quit (mainloop);
        exit (EXIT_FAILURE);
    }

    g_debug ("Creating common  gobject.");
    EasCommonObj = g_object_new (EAS_TYPE_COMMON , NULL);
    if (EasCommonObj == NULL)
    {
        g_debug ("Error: Failed to create common  instance");
        g_main_loop_quit (mainloop);
        exit (EXIT_FAILURE);
    }

    g_debug ("Creating mail  gobject.");
    EasMailObj = eas_mail_new ();
    if (EasMailObj == NULL)
    {
        g_debug ("Error: Failed to create common  instance");
        g_main_loop_quit (mainloop);
        exit (EXIT_FAILURE);
    }

    g_debug ("Pass a EasConnection handle to the exposed GObjects");
    // eas_sync_set_eas_connection(EasSyncObj, EasConnObj);
    // ret = eas_common_set_eas_connection(EasCommonObj, EasConnObj);
    // ret = eas_contact_set_eas_connection(EasContactObj, EasConnObj);
    // eas_mail_set_eas_connection(EasMailObj, EasConnObj);

    g_debug ("Connecting to the session DBus");
    bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
    if (error != NULL)
    {
        g_debug ("Error: Connecting to the session DBus (%s)", error->message);
        g_clear_error (&error);
        g_main_loop_quit (mainloop);
        exit (EXIT_FAILURE);
    }

    g_debug ("Registering the well-known name (%s)", EAS_SERVICE_NAME);
    busProxy = dbus_g_proxy_new_for_name (bus,
                                          DBUS_SERVICE_DBUS,
                                          DBUS_PATH_DBUS,
                                          DBUS_INTERFACE_DBUS);
    if (busProxy == NULL)
    {
        g_debug ("Error: Failed to get a proxy for D-Bus");
        g_main_loop_quit (mainloop);
        exit (EXIT_FAILURE);
    }

    /* register the well-known name.*/
    g_debug ("D-Bus RequestName RPC ");
    if (!dbus_g_proxy_call (busProxy,
                            "RequestName",
                            &error,
                            G_TYPE_STRING,
                            EAS_SERVICE_NAME,
                            G_TYPE_UINT,
                            0,
                            G_TYPE_INVALID,
                            G_TYPE_UINT,
                            &result,
                            G_TYPE_INVALID))
    {
        g_debug ("Error: D-Bus RequestName RPC failed (%s)", error->message);
        g_main_loop_quit (mainloop);
        exit (EXIT_FAILURE);
    }

    g_debug ("RequestName returned %d", result);
    if (result != 1)
    {
        g_debug ("Error: Failed to get the primary well-known name");
        exit (EXIT_FAILURE);
    }


    /*
     //we don't want EasConnObj to be exposed on DBus Interface anymore
        g_debug("Registering Gobjects on the D-Bus.");
        dbus_g_connection_register_g_object(bus,
                                          EAS_SERVICE_OBJECT_PATH,
                                          G_OBJECT(EasConnObj));

    */

    //  Registering  calendar Gobject
    dbus_g_connection_register_g_object (bus,
                                         EAS_SERVICE_SYNC_OBJECT_PATH,
                                         G_OBJECT (EasSyncObj));

    //  Registering  common Gobject
    dbus_g_connection_register_g_object (bus,
                                         EAS_SERVICE_COMMON_OBJECT_PATH,
                                         G_OBJECT (EasCommonObj));


    //  Registering  mail Gobject
    dbus_g_connection_register_g_object (bus,
                                         EAS_SERVICE_MAIL_OBJECT_PATH,
                                         G_OBJECT (EasMailObj));


    g_debug ("Ready to serve requests");

#ifndef DISABLE_EAS_DAEMON
    if (daemon (0, 0) != 0)
    {
        g_debug ("Failed to daemonize");
    }
#else
    g_debug ("Not daemonizing (built with DISABLE_EAS_DAEMON)");
#endif

    g_main_loop_run (mainloop);

    // Clean up
    g_main_loop_unref (mainloop);

    return 0;
}

