/*
 *  Filename: utils.h
 */

#ifndef _EAS_MAIL_UTILS_H_
#define _EAS_MAIL_UTILS_H_

#include <glib-object.h>

#define MAX_LEN_OF_INT32_AS_STRING	12

// creates a new string and populates with next 'field' from provided data (a field being the data between separators). 
// moves the data ptr to the start of the next field (or null terminator)
// eg data == ",1,Inbox,2" or "5,1,,2"
gchar* get_next_field(gchar **data, const gchar *separator);


// takes an array of (potentially empty/null) strings and num items in the array and concats the strings with separators
// eg "a,b,c"
gchar * strconcatwithseparator(gchar **strings, guint num, const gchar *sep);


#endif // _EAS_MAIL_UTILS_H_