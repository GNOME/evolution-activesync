/*
 *  Filename: eas-email-info.c
 */

#include <stdio.h>
#include <string.h>

#include "eas-email-info.h"
#include "eas-attachment.h"
#include "utils.h"

G_DEFINE_TYPE (EasEmailInfo, eas_email_info, G_TYPE_OBJECT);

static void
eas_email_info_init (EasEmailInfo *object)
{
	g_print("eas_email_info_init++");
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

	total_len += strlen(header->name);
	total_len += strlen(header->value);

	return total_len;
}

EasEmailInfo *
eas_email_info_new()
{
	g_print("eas_email_info_new++\n");	
	
	EasEmailInfo *object = NULL;

	object = g_object_new (EAS_TYPE_EMAIL_INFO , NULL);

	g_print("eas_email_info_new--\n");	
	
	return object;
}

gboolean 
eas_email_info_serialise(EasEmailInfo* this_g, gchar **result)
{
	gboolean ret = TRUE;
	// TODO change to expect caller to pass in memory
	
// TODO turn EasEmailInfo object into a null terminated string
	guint total_len = eas_email_info_serialised_length(this_g);
	
	// server_id:

	// email header tuples:

	// attachments:
	// allow for list size at front

	// get serialised length of each attachment (each includes a null terminator which we need to decrement)
	
	
	// flags:

	// categories:
	return ret;
	
}

gboolean 
eas_email_info_deserialise(EasEmailInfo* this_g, const gchar *data)
{
	gboolean ret = TRUE;
// TODO turn string into object
	return ret;
}

guint 
eas_email_info_serialised_length(EasEmailInfo *this_g)
{
	// calculate the total length:
	guint total_len = 0;
	guint list_len = 0;
	gchar list_size[MAX_LEN_OF_INT32_AS_STRING] = "";

	total_len += strlen(this_g->server_id);	// allow for separator

	// serialised length of headers list allowing for list size at front
	list_len = g_slist_length(this_g->headers);
	snprintf(list_size, sizeof(list_size)/sizeof(list_size[0]), "%d", list_len);
	total_len += strlen(list_size);

	GSList *l = NULL;	
	EasEmailHeader *header = NULL;
	for (l = this_g->headers; l != NULL; l = g_slist_next (l))
	{
		header = l->data;
		total_len += eas_mail_info_header_serialised_length(header);
	}

	EasAttachment *attachment = NULL;
	for (l = this_g->attachments; l != NULL; l = g_slist_next (l))
	{
		attachment = l->data;
		total_len += eas_attachment_serialised_length(attachment);
	}

	// serialise everything:
	
	return total_len;
}