/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#include "eas-utils.h"

// takes a null terminated array of strings. frees the strings pointed to and the array itself
void free_string_array (gchar **string_array)
{
    guint i = 0;
    if (!string_array)
        return;

    while (string_array[i])
    {
        g_free (string_array[i]);
        i++;
    }
    g_free (string_array);
}

// gets the number of items in a null terminated array (of strings)
guint array_length (const gchar **array)
{
    guint i = 0;

    for (i = 0; array[i] != NULL; i++);

    return i;
}