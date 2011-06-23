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

    for (i = 0; i < array_len - 1; i++)
    {
        EasFolder *folder;
        g_assert (l != NULL);
        folder = l->data;

        if (!eas_folder_serialise (folder, & (*serialised_folder_array) [i]))
        {
            g_debug ("failed!");
            ret = FALSE;
            goto cleanup;
        }

        l = g_slist_next (l);
    }

cleanup:
    if (!ret)
    {
        for (i = 0; i < array_len - 1; i++)
        {
            g_free ( (*serialised_folder_array) [i]);
        }
        g_free (*serialised_folder_array);
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     ("out of memory"));
    }

    return ret;
}

// allocates an array of ptrs to strings and the strings it points to and populates each string with serialised folder details
// null terminates the array
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
    if (!serialised_email_info_array)
    {
        ret = FALSE;
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     ("out of memory"));
        goto finish;
    }

    for (i = 0; i < array_len - 1; i++)
    {
        gchar *tstring = g_strdup (l->data);
        g_assert (l != NULL);
        (*serialised_email_info_array) [i] = tstring;
        l = g_slist_next (l);
    }

finish:
    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);
    }

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

    for (i = 0; i < array_len - 1; i++)
    {
        gchar *tstring = g_strdup (l->data);
        g_assert (l != NULL);
        (*serialised_cal_info_array) [i] = tstring;
        l = g_slist_next (l);
    }

    return ret;
}



