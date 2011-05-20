/*
 *  Filename: eas-attachment.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "eas-attachment.h"
#include "utils.h"

G_DEFINE_TYPE (EasAttachment, eas_attachment, G_TYPE_OBJECT);

const gchar *attachment_separator = ",";


static void
eas_attachment_init (EasAttachment *object)
{
	/* initialization code */
	object->file_reference = NULL;		
	object->estimated_size = 0;
	object->display_name = NULL;	
}

static void
eas_attachment_finalize (GObject *object)
{
	EasAttachment *this_g = (EasAttachment*)object;
	/* deinitalization code */
	g_free(this_g->file_reference);
	g_free(this_g->display_name);
	G_OBJECT_CLASS (eas_attachment_parent_class)->finalize (object);
}

static void
eas_attachment_class_init (EasAttachmentClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	// TODO better way to get rid of warnings about above 2 lines?
	void *temp = (void*)object_class;
	temp = (void*)parent_class;
	
	object_class->finalize = eas_attachment_finalize;
}

EasAttachment *
eas_attachment_new()
{
	g_print("eas_attachment_new++\n");	
	
	EasAttachment *object = NULL;

	object = g_object_new (EAS_TYPE_ATTACHMENT , NULL);

	g_print("eas_attachment_new--\n");	
	
	return object;
}

// TODO - change so that caller allocates memory for result
gboolean 
eas_attachment_serialise(EasAttachment *attachment, gchar **result)
{
	g_print("eas_attachment_serialise++\n");  
	gchar est_size[MAX_LEN_OF_INT32_AS_STRING] = "";			
	
	g_assert(attachment->estimated_size);	

	snprintf(est_size, sizeof(est_size)/sizeof(est_size[0]), "%d", attachment->estimated_size);
	
	gchar *strings[3] = {attachment->file_reference, attachment->display_name, est_size};

	*result = strconcatwithseparator(strings, sizeof(strings)/sizeof(strings[0]), attachment_separator);
	
	if(!*result)
	{
		g_print("eas_attachment_serialise--\n");
		return FALSE;
	}

	g_print("eas_attachment_serialise--\n");	
	return TRUE;
}	

gboolean 
eas_attachment_deserialise(EasAttachment *attachment, const gchar *data)
{
	// TODO - error handling (get_next_field can fail)
	g_print("eas_attachment_deserialise++\n");

	gboolean ret = TRUE;
	
	g_assert(attachment);
	g_assert(data);
	
	gchar *from = (gchar*)data;
	gchar *est_size = NULL;

	// file_reference
	if(attachment->file_reference != NULL)   //just in case
	{
		g_free(attachment->file_reference);
	}
	attachment->file_reference = get_next_field(&from, attachment_separator);	
	g_print("file_reference = %s\n", attachment->file_reference);
	
	// display_name
	if(attachment->display_name != NULL)
	{
		g_free(attachment->display_name);
	}	
	attachment->display_name = get_next_field(&from, attachment_separator);
	g_print("display name = %s\n", attachment->display_name);
	
	//estimated_size
	est_size = get_next_field(&from, attachment_separator);
	if(strlen(est_size))
	{
		attachment->estimated_size = atoi(est_size);
	}
	g_print("estimated_size = %d\n", attachment->estimated_size);
	
	g_print("eas_attachment_deserialise--\n");	

	return ret;	
}	


guint 
eas_attachment_serialised_length(EasAttachment *attachment)
{
	guint len = 0;

	// file_reference:
	g_assert(attachment->file_reference);
	len += strlen(attachment->file_reference) + 1;	// null-terminate allows for separator
	// display_name:
	if(attachment->display_name)	// optional field
	{
		len += strlen(attachment->display_name) + 1;
	}
	else
	{
		len += 1;	// just separator
	}
	// estimated_size:
	g_assert(attachment->estimated_size);	
	gchar est_size[MAX_LEN_OF_INT32_AS_STRING] = "";				
	snprintf(est_size, sizeof(est_size)/sizeof(est_size[0]), "%d", attachment->estimated_size);	

	len += strlen(est_size) +1;		// no separator at end, allows for null terminate

	g_print("eas_attachment_serialised_length returning %d\n", len);
	return len;
	
}
