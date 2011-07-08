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

#ifndef _EAS_MAIL_UTILS_H_
#define _EAS_MAIL_UTILS_H_

#include <glib-object.h>

#define MAX_LEN_OF_INT32_AS_STRING	12		// inc null terminator
#define MAX_LEN_OF_UINT8_AS_STRING	4		// inc null terminator

// creates a new string and populates with next 'field' from provided data (a field being the data between separators).
// moves the data ptr to the start of the next field (or null terminator)
// eg data == ",1,Inbox,2" or "5,1,,2"
gchar* get_next_field (gchar **data, const gchar *separator);

#endif // _EAS_MAIL_UTILS_H_
