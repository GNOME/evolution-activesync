/*
 * ActiveSync core protocol library
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

#include "eas-folder.h"
#include "../libeas/eas-connection.h"
#include "serialise_utils.h"


// allocates an array of ptrs to strings and the strings it points to and populates each string with serialised folder details
// null terminates the array
gboolean
build_serialised_folder_array (gchar ***serialised_folder_array, const GSList *folder_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;
	guint array_len = g_slist_length ( (GSList*) folder_list) + 1; //cast away const to avoid warning. +1 to allow terminating null
	GSList *l = (GSList*) folder_list;

	g_assert (serialised_folder_array);
	g_assert (*serialised_folder_array == NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	*serialised_folder_array = g_malloc0 (array_len * sizeof (gchar*));

	for (i = 0; i < array_len - 1; i++) {
		EasFolder *folder;
		g_assert (l != NULL);
		folder = l->data;

		if (!eas_folder_serialise (folder, & (*serialised_folder_array) [i])) {
			g_debug ("failed!");
			ret = FALSE;
			goto cleanup;
		}

		l = g_slist_next (l);
	}

cleanup:
	if (!ret) {
		for (i = 0; i < array_len - 1; i++) {
			g_free ( (*serialised_folder_array) [i]);
		}
		g_free (*serialised_folder_array);
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
	}

	return ret;
}

// creates a null terminated array of strings from a list of strings
gboolean
build_serialised_email_info_array (gchar ***serialised_email_info_array, const GSList *email_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;
	guint array_len = g_slist_length ( (GSList*) email_list) + 1;  //cast away const to avoid warning. +1 to allow terminating null
	GSList *l = (GSList*) email_list;

	g_debug ("build email arrays++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	g_assert (serialised_email_info_array);
	g_assert (*serialised_email_info_array == NULL);

	*serialised_email_info_array = g_malloc0 (array_len * sizeof (gchar*));
	if (!serialised_email_info_array) {
		ret = FALSE;
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		goto finish;
	}

	for (i = 0; i < array_len - 1; i++) {
		gchar *tstring = g_strdup (l->data);
		g_assert (l != NULL);
		(*serialised_email_info_array) [i] = tstring;
		l = g_slist_next (l);
	}

finish:
	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	g_debug ("build email arrays--");
	return ret;
}

// allocates an array of ptrs to strings and the strings it points to and populates each string with serialised folder details
// null terminates the array
gboolean
build_serialised_calendar_info_array (gchar ***serialised_cal_info_array, const GSList *cal_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;
	GSList *l = (GSList*) cal_list;
	guint array_len = g_slist_length ( (GSList*) cal_list) + 1; //cast away const to avoid warning. +1 to allow terminating null

	g_debug ("build cal arrays++");
	g_assert (serialised_cal_info_array);
	g_assert (*serialised_cal_info_array == NULL);

	*serialised_cal_info_array = g_malloc0 (array_len * sizeof (gchar*));

	for (i = 0; i < array_len - 1; i++) {
		gchar *tstring = g_strdup (l->data);
		g_assert (l != NULL);
		(*serialised_cal_info_array) [i] = tstring;
		l = g_slist_next (l);
	}

	return ret;
}



