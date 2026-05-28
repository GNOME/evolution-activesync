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
#include <gio/gio.h>
#include <stdlib.h>
#include <unistd.h>
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
static EasSync *g_eas_sync = NULL;
static EasCommon *g_eas_common = NULL;
static EasMail *g_eas_mail = NULL;
static EasTest *g_eas_test = NULL;

static void
on_bus_acquired (GDBusConnection *connection, const gchar *name G_GNUC_UNUSED, gpointer user_data G_GNUC_UNUSED)
{
	GError *error = NULL;

	g_debug ("Bus acquired, exporting objects");

	if (!g_dbus_interface_skeleton_export (eas_sync_get_skeleton (g_eas_sync),
					       connection, EAS_SERVICE_SYNC_OBJECT_PATH, &error) ||
	    !g_dbus_interface_skeleton_export (eas_common_get_skeleton (g_eas_common),
					       connection, EAS_SERVICE_COMMON_OBJECT_PATH, &error) ||
	    !g_dbus_interface_skeleton_export (eas_mail_get_skeleton (g_eas_mail),
					       connection, EAS_SERVICE_MAIL_OBJECT_PATH, &error) ||
	    !g_dbus_interface_skeleton_export (eas_test_get_skeleton (g_eas_test),
					       connection, EAS_SERVICE_TEST_OBJECT_PATH, &error)) {
		g_critical ("Failed to export D-Bus objects: %s", error->message);
		g_clear_error (&error);
		if (g_mainloop)
			g_main_loop_quit (g_mainloop);
	}
}

static void
on_name_acquired (GDBusConnection *connection G_GNUC_UNUSED, const gchar *name, gpointer user_data G_GNUC_UNUSED)
{
	g_debug ("Acquired D-Bus name: %s", name);
	g_debug ("Ready to serve requests");
}

static void
on_name_lost (GDBusConnection *connection G_GNUC_UNUSED, const gchar *name, gpointer user_data G_GNUC_UNUSED)
{
	g_warning ("Lost D-Bus name: %s", name);
	if (g_mainloop)
		g_main_loop_quit (g_mainloop);
}

int main (int argc, char** argv)
{
	GMainLoop* loop = NULL;
	guint owner_id;
	GError* error = NULL;

	g_log_set_default_handler (eas_logger, NULL);

	signal (SIGABRT, &signalHandler);
	signal (SIGTERM, &signalHandler);
	signal (SIGINT, &signalHandler);

	loop = g_main_loop_new (NULL, FALSE);
	if (loop == NULL) {
		g_debug ("Error: Couldn't create GMainLoop");
		exit (EXIT_FAILURE);
	}

	g_mainloop = loop;

	g_debug ("activesyncd Daemon Started");

	g_eas_sync = eas_sync_new ();
	g_eas_common = g_object_new (EAS_TYPE_COMMON, NULL);
	g_eas_mail = eas_mail_new ();
	g_eas_test = eas_test_new ();

	if (!g_eas_sync || !g_eas_common || !g_eas_mail || !g_eas_test) {
		g_critical ("Failed to create service objects");
		exit (EXIT_FAILURE);
	}

	owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
				   EAS_SERVICE_NAME,
				   G_BUS_NAME_OWNER_FLAGS_NONE,
				   on_bus_acquired,
				   on_name_acquired,
				   on_name_lost,
				   NULL, NULL);

#ifndef DISABLE_EAS_DAEMON
	if (daemon (0, 0) != 0)
		g_debug ("Failed to daemonize");
#else
	g_debug ("Not daemonizing (built with DISABLE_EAS_DAEMON)");
#endif

	g_main_loop_run (loop);

	g_debug ("Main Cleanup");
	g_mainloop = NULL;

	g_bus_unown_name (owner_id);

	g_dbus_interface_skeleton_unexport (eas_sync_get_skeleton (g_eas_sync));
	g_dbus_interface_skeleton_unexport (eas_common_get_skeleton (g_eas_common));
	g_dbus_interface_skeleton_unexport (eas_mail_get_skeleton (g_eas_mail));
	g_dbus_interface_skeleton_unexport (eas_test_get_skeleton (g_eas_test));

	g_clear_object (&g_eas_sync);
	g_clear_object (&g_eas_common);
	g_clear_object (&g_eas_mail);
	g_clear_object (&g_eas_test);

	g_main_loop_unref (loop);

	g_debug ("Exiting main()");
	return 0;
}
