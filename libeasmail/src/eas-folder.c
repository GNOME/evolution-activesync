/*
 * ActiveSync client library for email access
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "eas-folder.h"
#include "utils.h"

G_DEFINE_TYPE (EasFolder, eas_folder, G_TYPE_OBJECT);

const gchar *folder_separator = "\n";

static void
eas_folder_init (EasFolder *object)
{
    object->parent_id = NULL;
    object->folder_id = NULL;
    object->display_name = NULL;
}

static void
eas_folder_finalize (GObject *object)
{
    EasFolder *self = (EasFolder *) object;

    g_free (self->parent_id);
    g_free (self->folder_id);
    g_free (self->display_name);

    G_OBJECT_CLASS (eas_folder_parent_class)->finalize (object);
}

static void
eas_folder_class_init (EasFolderClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = eas_folder_finalize;
}




EasFolder *
eas_folder_new()
{
    EasFolder *object = NULL;
    object = g_object_new (EAS_TYPE_FOLDER , NULL);
    return object;
}

// take the contents of the object and turn it into a null terminated string
// fields are separated by a separator (eg ',') and there is a trailing separator
// empty fields included, eg "5,1,Inbox,2," or ",1,,,"
gboolean
eas_folder_serialise (EasFolder* folder, gchar **result)
{
    gboolean ret = TRUE;
    gchar type[4] = "";
    gchar *strings[5] = {folder->parent_id, folder->folder_id, folder->display_name, type, 0};

    g_assert (result);
    g_assert (*result == NULL);

    // Bad assert?
    //  g_assert(folder->type < EAS_FOLDER_TYPE_MAX);

    if (folder->type)
    {
        //itoa not standard/supported on linux?
        snprintf (type, sizeof (type) / sizeof (type[0]), "%d", folder->type);
    }

    *result = g_strjoinv (folder_separator, strings);

    if (!*result)
    {
        ret = FALSE;
    }

    return (*result ? TRUE : FALSE);
}


// populate the folder object from a null terminated string eg ",1,,,".
gboolean
eas_folder_deserialise (EasFolder* folder, const gchar *data)
{
    gboolean ret = TRUE;
    gchar *from = (gchar*) data;
    gchar *type = NULL;

    g_assert (folder);
    g_assert (data);

    // parent_id
    if (folder->parent_id != NULL)  //expect empty structure, but just in case
    {
        g_free (folder->parent_id);
    }
    folder->parent_id = get_next_field (&from, folder_separator);
    if (!folder->parent_id)
    {
        ret = FALSE;
        goto cleanup;
    }

    // server_id
    if (folder->folder_id != NULL)
    {
        g_free (folder->folder_id);
    }
    folder->folder_id = get_next_field (&from, folder_separator);
    if (!folder->folder_id)
    {
        ret = FALSE;
        goto cleanup;
    }

    // display_name
    if (folder->display_name != NULL)
    {
        g_free (folder->display_name);
    }
    folder->display_name = get_next_field (&from, folder_separator);
    if (!folder->display_name)
    {
        ret = FALSE;
        goto cleanup;
    }

    //type
    type = get_next_field (&from, folder_separator);
    if (!type)
    {
        ret = FALSE;
        goto cleanup;
    }
    if (strlen (type))
    {
        folder->type = atoi (type);
        g_free (type);
    }

cleanup:
    if (!ret)
    {
        g_free (folder->parent_id);
        folder->parent_id = NULL;
        g_free (folder->folder_id);
        folder->folder_id = NULL;
        g_free (folder->display_name);
        folder->display_name = NULL;
    }

    return ret;
}

