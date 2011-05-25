/*
 *  Filename: eas-email-info.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eas-email-info.h"
#include "eas-attachment.h"
#include "utils.h"

G_DEFINE_TYPE (EasEmailInfo, eas_email_info, G_TYPE_OBJECT);

const gchar *sep = ",";

static void
eas_email_info_init (EasEmailInfo *object)
{
	g_debug("eas_email_info_init++");
	/* initialization code */
	object->server_id = NULL;	
	object->headers = NULL;
	object->attachments = NULL;
	object->categories = NULL;
	object->flags = 0;

}

static void
eas_email_info_finalize (GObject *object)
{
	EasEmailInfo *this_g = (EasEmailInfo*)object;
	/* deinitalization code */
	g_free(this_g->server_id);	
	g_slist_foreach(this_g->headers, (GFunc) g_free, NULL); 
	g_free(this_g->headers);
	g_slist_foreach(this_g->attachments, (GFunc) g_object_unref, NULL);
	g_slist_free(this_g->attachments);  // list of EasAttachments
	g_slist_foreach(this_g->categories, (GFunc) g_free, NULL);
	g_slist_free(this_g->categories);
	
	G_OBJECT_CLASS (eas_email_info_parent_class)->finalize (object);
}

static void
eas_email_info_class_init (EasEmailInfoClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	// TODO better way to get rid of warnings about above 2 lines?
	void *temp = (void*)object_class;
	temp = (void*)parent_class;

	object_class->finalize = eas_email_info_finalize;
}

// returns the length of a serialised header (eg "To, Lorna McNeill") including the null terminator
static guint
eas_mail_info_header_serialised_length(EasEmailHeader *header)
{
	guint total_len = 0;

	total_len += strlen(header->name) + 1;
	total_len += strlen(header->value) + 1;

	return total_len;
}

EasEmailInfo *
eas_email_info_new()
{
	g_debug("eas_email_info_new++");	
	
	EasEmailInfo *object = NULL;

	object = g_object_new (EAS_TYPE_EMAIL_INFO , NULL);

	g_debug("eas_email_info_new--");	
	
	return object;
}


//returns the length of the data when serialised (including null terminator)
static guint 
eas_email_info_serialised_length(EasEmailInfo *this_g)
{
	g_debug("eas_email_info_serialised_length++");
	// calculate the total length:
	guint total_len = 0;
	guint list_len = 0;
	gchar list_size[MAX_LEN_OF_INT32_AS_STRING] = "";
	GSList *l = NULL;	
	EasEmailHeader *header = NULL;
	
	// server id
	total_len += strlen(this_g->server_id) + 1;	// allow for separator
	g_debug("total_len = %d", total_len);

	//headers
	// serialised length of headers list allowing for list size at front
	list_len = g_slist_length(this_g->headers);
	snprintf(list_size, sizeof(list_size)/sizeof(list_size[0]), "%d", list_len);
	total_len += strlen(list_size) + 1;

	for (l = this_g->headers; l != NULL; l = g_slist_next (l))
	{
		header = l->data;
		total_len += eas_mail_info_header_serialised_length(header);
	}

//attachments
	list_len = g_slist_length(this_g->attachments);
	snprintf(list_size, sizeof(list_size)/sizeof(list_size[0]), "%d", list_len);
	total_len += strlen(list_size) + 1;
	
	EasAttachment *attachment = NULL;
	for (l = this_g->attachments; l != NULL; l = g_slist_next (l))
	{
		attachment = l->data;
		total_len += eas_attachment_serialised_length(attachment);
	}

//flags
	gchar flags_as_string[MAX_LEN_OF_UINT8_AS_STRING] = "";
	snprintf(flags_as_string, sizeof(flags_as_string)/sizeof(flags_as_string[0]), "%d", this_g->flags);
	total_len += strlen(flags_as_string) + 1;

//categories	
	list_len = g_slist_length(this_g->categories);
	snprintf(list_size, sizeof(list_size)/sizeof(list_size[0]), "%d", list_len);
	total_len += strlen(list_size) + 1;

	gchar *category = NULL;
	for (l = this_g->categories; l != NULL; l = g_slist_next (l))
	{
		category = l->data;
		total_len += strlen(category) + 1;
	}

	g_debug("total_len = %d", total_len);
	
	g_debug("eas_email_info_serialised_length--");	
	return total_len;
}	

gboolean 
eas_email_info_serialise(EasEmailInfo* this_g, gchar **result)
{
	g_debug("eas_email_info_serialise++");
	gboolean ret = TRUE;
	gchar list_size[MAX_LEN_OF_INT32_AS_STRING] = "";	
	guint list_len = 0;
	GSList *l = NULL;
	
	// turn EasEmailInfo object into a null terminated string
	guint total_len = eas_email_info_serialised_length(this_g);

	g_debug("total length of serialised email info = %d", total_len);
	// allocate the memory to hold the serialised object:
	*result = (gchar*)g_malloc0((total_len * sizeof(gchar)));
	if(!(*result))
	{
		g_debug("memory allocation failure!");
		ret = FALSE;
	}
	else
	{
		gchar *out = *result;
	
		// serialise everything:
		//server_id
		g_debug("serialising serverid");
		out = g_stpcpy(out, this_g->server_id);	//g_stpcpy copies a nul-terminated string into the dest buffer, include the trailing nul, and return a pointer to the trailing nul byte
		out = g_stpcpy(out, sep);
		
		//headers
		g_debug("serialising headers");
		list_len = g_slist_length(this_g->headers);
		snprintf(list_size, sizeof(list_size)/sizeof(list_size[0]), "%d", list_len);
		out = g_stpcpy(out, list_size);
		out = g_stpcpy(out, sep);
			
		EasEmailHeader *header = NULL;
		for (l = this_g->headers; l != NULL; l = g_slist_next (l))
		{
			header = l->data;
			out = g_stpcpy(out, header->name);
			out = g_stpcpy(out, sep);
			out = g_stpcpy(out, header->value);	
			out = g_stpcpy(out, sep);
		}	
		//attachments
		g_debug("serialising attachments");
		list_len = g_slist_length(this_g->attachments);
		snprintf(list_size, sizeof(list_size)/sizeof(list_size[0]), "%d", list_len);
		out = g_stpcpy(out, list_size);
		out = g_stpcpy(out, sep);		
		EasAttachment *attachment = NULL;
		gchar *temp = NULL;
		for (l = this_g->attachments; l != NULL; l = g_slist_next (l))
		{
			attachment = l->data;
			
			if(!eas_attachment_serialise(attachment, &temp))
			{
				ret = FALSE;
			}
			else
			{
				out = g_stpcpy(out, temp);
				g_free(temp);
				temp = NULL;
				if(g_slist_next (l))	// don't add a trailing separator
				{
					out = g_stpcpy(out, sep);
				}				
			}
		}	
		//flags
		g_debug("serialising flags");		
		gchar flags_as_string[MAX_LEN_OF_UINT8_AS_STRING] = "";
		snprintf(flags_as_string, sizeof(flags_as_string)/sizeof(flags_as_string[0]), "%d", this_g->flags);		
		out = g_stpcpy(out, flags_as_string);
		out = g_stpcpy(out, sep);		
		//categories
		g_debug("serialising categories");		
		list_len = g_slist_length(this_g->categories);

		snprintf(list_size, sizeof(list_size)/sizeof(list_size[0]), "%d", list_len);
		out = g_stpcpy(out, list_size);
		if(list_len)
		{
			out = g_stpcpy(out, sep);	
		}
		gchar *category = NULL;
		for (l = this_g->categories; l != NULL; l = g_slist_next (l))
		{
			category = l->data;
			out = g_stpcpy(out, category);
			if(g_slist_next (l))	// don't add a trailing separator
			{
				out = g_stpcpy(out, sep);
			}
		}
	}

	if(!ret)
	{
		g_debug("failed!");
		g_free(*result);
		*result = NULL;
	}

	g_debug("eas_email_info_serialise--");
	return ret;
}
	

gboolean 
eas_email_info_deserialise(EasEmailInfo* this_g, const gchar *data)
{
	// TODO proper error handling - eg deal with get_next_field returning null
	g_debug("eas_email_info_deserialise++");
	gboolean ret = TRUE;
	guint list_len = 0, i = 0;
	gchar *list_len_as_string = NULL;
	EasEmailHeader *header = NULL;
	EasAttachment *attachment = NULL;
	GSList *headers = NULL;
	GSList *attachments = NULL;
	GSList *categories = NULL;
	
	g_assert(this_g);
	g_assert(data);
	
	gchar *from = (gchar*)data;
	// turn string into object
	// server_id
	if(this_g->server_id != NULL)   //just in case
	{
		g_free(this_g->server_id);
	}
	this_g->server_id = get_next_field(&from, sep);	
	if(!this_g->server_id)
	{
		ret = FALSE;
		goto cleanup;
	}
	g_debug("server_id = %s", this_g->server_id);

	//headers
	list_len_as_string = get_next_field(&from, sep);
	list_len = atoi(list_len_as_string);
	
	g_debug("%d headers", list_len);

	for(i = 0; i < list_len; i++)
	{
		header = g_malloc0(sizeof(EasEmailHeader));
		header->name = get_next_field(&from, sep);
		if(!header->name)
		{
			ret = FALSE;
			goto cleanup;
		}
		header->value = get_next_field(&from, sep);
		if(!header->value)
		{
			ret = FALSE;
			goto cleanup;
		}
		headers = g_slist_append(headers, header);
	}
	this_g->headers = headers;
	
	//attachments
	list_len_as_string = get_next_field(&from, sep);
	if(!list_len_as_string)
	{
		ret = FALSE;
		goto cleanup;
	}
	list_len = atoi(list_len_as_string);
	g_debug("%d attachments", list_len);
	for(i = 0; i < list_len; i++)
	{
		attachment = g_malloc0(sizeof(EasAttachment));
		if(!eas_attachment_deserialise(attachment, from))
		{
			ret = FALSE;
			goto cleanup;
		}
		from += eas_attachment_serialised_length (attachment);//attachment deserialise doesn't move pointer along

		attachments = g_slist_append(attachments, attachment);
	}
	this_g->attachments = attachments;
	
	//flags
	gchar *flags_as_string = get_next_field(&from, sep);
	if(!flags_as_string)
	{
		ret = FALSE;
		goto cleanup;
	}
	if(strlen(flags_as_string))
	{
		this_g->flags = atoi(flags_as_string);
	}
	g_debug("flags = %d", this_g->flags);	
	
	//categories
	list_len_as_string = get_next_field(&from, sep);
	list_len = atoi(list_len_as_string);
	g_debug("%d categories", list_len);	

	for(i = 0; i < list_len; i++)
	{
		gchar *category = get_next_field(&from, sep);
		if(!category)
		{
			ret = FALSE;
			goto cleanup;
		}
		categories = g_slist_append(categories, category);
	}	
	this_g->categories = categories;

cleanup:
	if(!ret)
	{
		g_debug("failed!");
		//TODO cleanup
	}

	g_debug("eas_email_info_deserialise--");
	return ret;
}

