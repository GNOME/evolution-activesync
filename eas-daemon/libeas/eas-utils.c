/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 *
 * code is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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