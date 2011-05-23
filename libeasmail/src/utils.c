/*
 *  Filename: utils.c
 */
#include <stdio.h>
#include <string.h>

#include "utils.h"

// returns a pointer to the next field. If field is empty will pass back an empty string. Only returns NULL in case of error
gchar* 
get_next_field(gchar **data, const gchar *separator)
{
	gchar *result = NULL, *to = *data;
	guint len = 0;  // length of string 

	while(*to && (*to != *separator))
	{
		to++;
	}
	len = (to - *data);

	result = (gchar*)g_malloc0((len * sizeof(gchar)) + 1 );	// allow for null terminate
	if(result)
	{
		strncpy(result, (*data), len);
		result[len] = 0;
		*data += len + 1;
	}
	
	g_print("get_next_field result = %s\n", result);
	return result;
}


gchar * 
strconcatwithseparator(gchar **strings, guint num, const gchar *sep)
{
	gchar *out = NULL;
	guint len = 0;  //length of resulting string in bytes
	int i;
		
	for(i=0; i<num; i++)
	{
		if(strings[i])
		{
			len = len + strlen(strings[i]);
		}
		len = len + strlen(sep);
	}

	out = (gchar*)g_malloc0((len * sizeof(gchar)));
	
	if(out)
	{
		for(i=0; i<num; i++)
		{
			if(strings[i])
			{
				g_print("strcat %s to %s\n", strings[i], out);
				strcat(out, strings[i]);
			}
			if(i<num)
			{
				strcat(out, sep);
			}
		}
	}
	
	return out;
}
