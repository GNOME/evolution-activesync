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

const gchar *sep = "\n";

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
	g_debug("eas_email_info_init--");
}

static void
eas_email_free_header(EasEmailHeader *header)
{
	g_free(header->name);
	g_free(header->value);
	g_free(header);
}

static void
eas_email_info_finalize (GObject *object)
{
	EasEmailInfo *self = (EasEmailInfo*)object;
    
	g_debug("eas_email_info_finalize++");
	/* deinitalization code */
	g_free(self->server_id);	
	
	g_slist_foreach(self->headers, (GFunc)eas_email_free_header, NULL); 
	g_slist_free(self->headers);

	g_slist_foreach(self->attachments, (GFunc)g_object_unref, NULL);

	g_slist_free(self->attachments);  // list of EasAttachments
	
	g_slist_foreach(self->categories, (GFunc) g_free, NULL);
	g_slist_free(self->categories);
	
	G_OBJECT_CLASS (eas_email_info_parent_class)->finalize (object);
	g_debug("eas_email_info_finalize--");
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
	EasEmailInfo *object = NULL;
	g_debug("eas_email_info_new++");

	object = g_object_new (EAS_TYPE_EMAIL_INFO , NULL);

	g_debug("eas_email_info_new--");	

	return object;
}


//returns the length of the data when serialised (including null terminator)
static guint 
eas_email_info_serialised_length(EasEmailInfo *self)
{
	// calculate the total length:
	guint total_len = 0;
	guint list_len = 0;
	gchar list_size[MAX_LEN_OF_INT32_AS_STRING] = "";
	GSList *l = NULL;	
	EasEmailHeader *header = NULL;
	EasAttachment *attachment = NULL;
	gchar flags_as_string[MAX_LEN_OF_UINT8_AS_STRING] = "";
	gchar *category = NULL;
    
	g_debug("eas_email_info_serialised_length++");
	
	// server id
	total_len += strlen(self->server_id) + 1;	// allow for separator

	//headers
	// serialised length of headers list allowing for list size at front
	list_len = g_slist_length(self->headers);
	snprintf(list_size, sizeof(list_size)/sizeof(list_size[0]), "%d", list_len);
	total_len += strlen(list_size) + 1;

	for (l = self->headers; l != NULL; l = g_slist_next (l))
	{
		header = l->data;
		total_len += eas_mail_info_header_serialised_length(header);
	}
	//attachments
	list_len = g_slist_length(self->attachments);
	snprintf(list_size, sizeof(list_size)/sizeof(list_size[0]), "%d", list_len);
	total_len += strlen(list_size) + 1;
	
	for (l = self->attachments; l != NULL; l = g_slist_next (l))
	{
		attachment = l->data;
		total_len += eas_attachment_serialised_length(attachment);
	}
//flags
	snprintf(flags_as_string, sizeof(flags_as_string)/sizeof(flags_as_string[0]), "%d", self->flags);
	total_len += strlen(flags_as_string) + 1;

//categories	
	list_len = g_slist_length(self->categories);
	snprintf(list_size, sizeof(list_size)/sizeof(list_size[0]), "%d", list_len);
	total_len += strlen(list_size) + 1;

	for (l = self->categories; l != NULL; l = g_slist_next (l))
	{
		category = l->data;
		total_len += strlen(category) + 1;
	}

	g_debug("total_len = %d", total_len);
	g_debug("eas_email_info_serialised_length--");	
	return total_len;
}	

gboolean 
eas_email_info_serialise(EasEmailInfo* self, gchar **result)
{
	gboolean ret = TRUE;
	gchar list_size[MAX_LEN_OF_INT32_AS_STRING] = "";	
	guint list_len = 0;
	GSList *l = NULL;
	
	// turn EasEmailInfo object into a null terminated string
	guint total_len = eas_email_info_serialised_length(self);
    
	g_debug("eas_email_info_serialise++");

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
		EasEmailHeader *header = NULL;
		EasAttachment *attachment = NULL;
		gchar *temp = NULL;
		gchar flags_as_string[MAX_LEN_OF_UINT8_AS_STRING] = "";
		gchar *category = NULL;
	
		// serialise everything:
		//server_id
		g_debug("serialising serverid");
		out = g_stpcpy(out, self->server_id);	//g_stpcpy copies a nul-terminated string into the dest buffer, include the trailing nul, and return a pointer to the trailing nul byte
		out = g_stpcpy(out, sep);
		
		//headers
		g_debug("serialising headers");
		list_len = g_slist_length(self->headers);
		snprintf(list_size, sizeof(list_size)/sizeof(list_size[0]), "%d", list_len);
		out = g_stpcpy(out, list_size);
		out = g_stpcpy(out, sep);
			
		for (l = self->headers; l != NULL; l = g_slist_next (l))
		{
			header = l->data;
			out = g_stpcpy(out, header->name);
			out = g_stpcpy(out, sep);
			out = g_stpcpy(out, header->value);	
			out = g_stpcpy(out, sep);
		}	
		//attachments
		g_debug("serialising attachments");
		list_len = g_slist_length(self->attachments);
		snprintf(list_size, sizeof(list_size)/sizeof(list_size[0]), "%d", list_len);
		out = g_stpcpy(out, list_size);
		out = g_stpcpy(out, sep);		
		for (l = self->attachments; l != NULL; l = g_slist_next (l))
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
		snprintf(flags_as_string, sizeof(flags_as_string)/sizeof(flags_as_string[0]), "%d", self->flags);		
		out = g_stpcpy(out, flags_as_string);
		out = g_stpcpy(out, sep);		
		//categories
		g_debug("serialising categories");		
		list_len = g_slist_length(self->categories);

		snprintf(list_size, sizeof(list_size)/sizeof(list_size[0]), "%d", list_len);
		out = g_stpcpy(out, list_size);
		if(list_len)
		{
			out = g_stpcpy(out, sep);	// don't add a separator if there aren't any attachments to follow
		}
		for (l = self->categories; l != NULL; l = g_slist_next (l))
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
eas_email_info_deserialise(EasEmailInfo* self, const gchar *data)
{
	// TODO proper error handling - eg deal with get_next_field returning null
	gboolean ret = TRUE;
	guint list_len = 0, i = 0;
	gchar *list_len_as_string = NULL;
	EasEmailHeader *header = NULL;
	EasAttachment *attachment = NULL;
	GSList *headers = NULL;
	GSList *attachments = NULL;
	GSList *categories = NULL;
	gchar *from = (gchar*)data;
    gchar *flags_as_string = NULL;
    
    g_debug("eas_email_info_deserialise++");
	g_assert(self);
	g_assert(data);
	
	// turn string into object
	// server_id
	if(self->server_id != NULL)   //just in case
	{
		g_free(self->server_id);
	}
	self->server_id = get_next_field(&from, sep);	
	if(!self->server_id)
	{
		ret = FALSE;
		goto cleanup;
	}
	g_debug("server_id = %s", self->server_id);

	//headers
	list_len_as_string = get_next_field(&from, sep);
	list_len = atoi(list_len_as_string);
	g_free(list_len_as_string);
	list_len_as_string = NULL;
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
	self->headers = headers;
	
	//attachments
	list_len_as_string = get_next_field(&from, sep);
	if(!list_len_as_string)
	{
		ret = FALSE;
		goto cleanup;
	}
	list_len = atoi(list_len_as_string);
	g_free(list_len_as_string);
	list_len_as_string = NULL;
	g_debug("%d attachments", list_len);
	for(i = 0; i < list_len; i++)
	{
		attachment = eas_attachment_new ();
		if(!eas_attachment_deserialise(attachment, from))
		{
			ret = FALSE;
			goto cleanup;
		}
		from += eas_attachment_serialised_length (attachment);//attachment deserialise doesn't move pointer along

		attachments = g_slist_append(attachments, attachment);
	}
	self->attachments = attachments;
	
	//flags
	flags_as_string = get_next_field(&from, sep);
	if(!flags_as_string)
	{
		ret = FALSE;
		goto cleanup;
	}
	if(strlen(flags_as_string))
	{
		self->flags = atoi(flags_as_string);
	}
	g_free(flags_as_string);
	flags_as_string = NULL;
	g_debug("flags = %d", self->flags);	
	
	//categories
	list_len_as_string = get_next_field(&from, sep);
	list_len = atoi(list_len_as_string);
	g_free(list_len_as_string);
	list_len_as_string = NULL;
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
	self->categories = categories;

cleanup:
	
	if(!ret)
	{
		g_debug("failed!");
		//TODO cleanup
	}

	g_debug("eas_email_info_deserialise--");
	return ret;
}

