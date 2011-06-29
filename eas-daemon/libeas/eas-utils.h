/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _EAS_UTILS_H_
#define _EAS_UTILS_H_

#include <glib-object.h>

/* Return the number of items in a null-terminated array
 *
 * @param[in] array
 *      A null-terminated array
 * @return Number of items in the array
 */
guint array_length(const gchar **array);

/* Free a null terminated array of strings (including the strings pointed to)
 *
 * @param[in] array
 *      A null-terminated array of strings
 */
void free_string_array(gchar **string_array);

#endif  // _EAS_UTILS_H_