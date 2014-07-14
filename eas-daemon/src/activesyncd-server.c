/*
 * ActiveSync DBus dæmon
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This file is provided under a dual Apache/LGPLv2.1 licence.  When
 * using or redistributing this file, you may do so under either
 * licence.
 *
 *
 * LGPLv2.1 LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later
 *   version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free
 *   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301 USA
 *
 *
 * APACHE LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

//system include
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <libsoup/soup.h>
#include <signal.h>

//user include
#include "activesyncd-common-defs.h"
#include "../libeas/eas-connection.h"
#include "eas-sync.h"
#include "eas-common.h"
#include "eas-mail.h"
#include "eas-test.h"

#include <eas-logger.h>

#ifdef DISABLE_EAS_DAEMON
#define dbg(fmtstr, args...) (g_debug(":%s: " fmtstr "", __func__, ##args))
#else
#define dbg(dummy...)
#endif

void signalHandler (int sig);
GMainLoop* g_mainloop = NULL;

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

	if (getenv ("EAS_DEBUG_FILE")) {
		logfile = fopen (g_getenv ("EAS_DEBUG_FILE"), "a");
	}

	if (getenv ("EAS_DEBUG")) {
		envLevel = atoi (g_getenv ("EAS_DEBUG"));
	}

	if (log_level == G_LOG_LEVEL_ERROR && g_default_logger) {
		g_default_logger (log_domain, log_level, message, user_data);
		return;
	}

	if (logfile) {
		fprintf (logfile, "(process:%d): ", pid);

		if (log_domain) {
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
	} else {
		printf ("(process:%d): ", pid);

		if (log_domain) {
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

void signalHandler (int sig)
{
	if (g_mainloop) {
		// Only quit the loop once. Doing it again
		// leads to a race condition (main() unrefs
		// loop, we use it again here).
		g_main_loop_quit (g_mainloop);
		g_mainloop = NULL;
	}
}

/*
  activesyncd entry point
*/
int main (int argc, char** argv)
{
	DBusGConnection* bus = NULL;
	DBusGProxy* busProxy = NULL;
	EasSync* EasSyncObj = NULL;
	EasCommon* EasCommonObj = NULL;
	EasMail*EasMailObj = NULL;
	EasTest* EasTestObj = NULL;
	GMainLoop* loop = NULL;

	guint result;
	GError* error = NULL;

#if !GLIB_CHECK_VERSION(2,36,0)
	g_type_init();
#endif
	dbus_g_thread_init();
#if 0
	g_log_set_handler (NULL,
			   G_LOG_LEVEL_DEBUG | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL,
			   eas_logger,
			   NULL);
#endif
	g_log_set_default_handler (eas_logger, NULL);

	signal (SIGABRT, &signalHandler);
	signal (SIGTERM, &signalHandler);
	signal (SIGINT, &signalHandler);

	loop = g_main_loop_new (NULL, FALSE);
	if (loop == NULL) {
		g_debug ("Error: Couldn't create GMainLoop");
		exit (EXIT_FAILURE);
	}

	// Give signalHandler() access to the main loop.
	g_mainloop = loop;

	//Creating all the GObjects
	g_debug ("activesyncd Daemon Started");

	g_debug ("Creating eas_sync  gobject.");
	EasSyncObj = eas_sync_new();
	if (EasSyncObj == NULL) {
		g_debug ("Error: Failed to create calendar  instance");
		g_main_loop_quit (loop);
		exit (EXIT_FAILURE);
	}

	g_debug ("Creating common  gobject.");
	EasCommonObj = g_object_new (EAS_TYPE_COMMON , NULL);
	if (EasCommonObj == NULL) {
		g_debug ("Error: Failed to create common  instance");
		g_main_loop_quit (loop);
		exit (EXIT_FAILURE);
	}

	g_debug ("Creating mail  gobject.");
	EasMailObj = eas_mail_new ();
	if (EasMailObj == NULL) {
		g_debug ("Error: Failed to create common  instance");
		g_main_loop_quit (loop);
		exit (EXIT_FAILURE);
	}

	EasTestObj = eas_test_new ();
	if (NULL == EasTestObj) {
		g_debug ("Failed to make EasTest instance");
		g_main_loop_quit (loop);
		exit (EXIT_FAILURE);
	}

	g_debug ("Connecting to the session DBus");
	bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_debug ("Error: Connecting to the session DBus (%s)", error->message);
		g_clear_error (&error);
		g_main_loop_quit (loop);
		exit (EXIT_FAILURE);
	}

	g_debug ("Registering the well-known name (%s)", EAS_SERVICE_NAME);
	busProxy = dbus_g_proxy_new_for_name (bus,
					      DBUS_SERVICE_DBUS,
					      DBUS_PATH_DBUS,
					      DBUS_INTERFACE_DBUS);
	if (busProxy == NULL) {
		g_debug ("Error: Failed to get a proxy for D-Bus");
		g_main_loop_quit (loop);
		exit (EXIT_FAILURE);
	}

	dbus_g_proxy_set_default_timeout (busProxy, 1000000);

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
				G_TYPE_INVALID)) {
		g_debug ("Error: D-Bus RequestName RPC failed (%s)", error->message);
		g_clear_error (&error);
		g_main_loop_quit (loop);
		exit (EXIT_FAILURE);
	}

	g_debug ("RequestName returned %d", result);
	if (result != 1) {
		g_debug ("Error: Failed to get the primary well-known name");
		exit (EXIT_FAILURE);
	}
	
	//  Registering  sync Gobject
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

	dbus_g_connection_register_g_object (bus,
					     EAS_SERVICE_TEST_OBJECT_PATH,
					     G_OBJECT (EasTestObj));

	g_debug ("Ready to serve requests");

#ifndef DISABLE_EAS_DAEMON
	if (daemon (0, 0) != 0) {
		g_debug ("Failed to daemonize");
	}
#else
	g_debug ("Not daemonizing (built with DISABLE_EAS_DAEMON)");
#endif

	g_main_loop_run (loop);

	// Clean up
	g_debug ("Main Cleanup");
	g_mainloop = NULL;
	g_main_loop_unref (loop);

	// clean up dbus and all its objects
	if (EasSyncObj) {
		g_debug ("Unregister and unref EasSyncObj");
		dbus_g_connection_unregister_g_object (bus, G_OBJECT (EasSyncObj));
		g_object_unref(EasSyncObj);
	}
	
	if (EasCommonObj) {
		g_debug ("Unregister and unref EasCommonObj");
		dbus_g_connection_unregister_g_object (bus, G_OBJECT (EasCommonObj));
		g_object_unref(EasCommonObj);
	}

	if (EasMailObj) {
		g_debug ("Unregister and unref EasMailObj");
		dbus_g_connection_unregister_g_object (bus, G_OBJECT (EasMailObj));
		g_object_unref(EasMailObj);
	}

	if (EasTestObj) {
		g_debug ("Unregister and unref EasTestObj");
		dbus_g_connection_unregister_g_object (bus, G_OBJECT (EasTestObj));
		g_object_unref(EasTestObj);
	}
	
	if (busProxy) {
		g_debug ("Unref busProxy");
		g_object_unref(busProxy);
	}

	if(bus)
	{
		g_debug ("Flush and unref DBusConnection bus");
		dbus_g_connection_flush (bus);
		dbus_g_connection_unref(bus);
	}

	g_debug ("Exiting main()");
	return 0;
}
