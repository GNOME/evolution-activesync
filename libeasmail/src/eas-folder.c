/*
 *  Filename: eas-folder.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "eas-folder.h"
#include "utils.h"

G_DEFINE_TYPE (EasFolder, eas_folder, G_TYPE_OBJECT);

const gchar *folder_separator = ",";

static void
eas_folder_init (EasFolder *object)
{
	g_print("eas_folder_init++\n");
	/* initialization code */
	object->parent_id = NULL;
	object->folder_id = NULL;
	object->display_name = NULL;
	
	g_print("eas_folder_init--\n");
}

static void
eas_folder_finalize (GObject *object)
{
	g_print("eas_folder_finalize++\n");
	/* deinitalization code here */
	EasFolder *this_g = (EasFolder *) object;

	g_free(this_g->parent_id);
	g_free(this_g->folder_id);
	g_free(this_g->display_name);

	G_OBJECT_CLASS (eas_folder_parent_class)->finalize (object);

	g_print("eas_folder_finalize--\n");
}

static void
eas_folder_class_init (EasFolderClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	// get rid of warnings about above 2 lines
	void *temp = (void*)object_class;
	temp = (void*)parent_class;
	
	object_class->finalize = eas_folder_finalize;
}

 


EasFolder *
eas_folder_new()
{
	g_print("eas_folder_new++\n");	
	
	EasFolder *object = NULL;

	object = g_object_new (EAS_TYPE_FOLDER , NULL);

	g_print("eas_folder_new--\n");	
	
	return object;
}

// take the contents of the object and turn it into a null terminated string
// fields are separated by a separator (eg ',') and there is a trailing separator
// empty fields included, eg "5,1,Inbox,2," or ",1,,,"
gboolean 
eas_folder_serialise(EasFolder* folder, gchar **result)
{
	gboolean ret = TRUE;
	
	g_print("eas_folder_serialise++\n");  
	gchar type[4] = "";

    g_assert(result);
    g_assert(*result == NULL);

    // Bad assert these numbers can change based on server whim
//	g_assert(folder->type < EAS_FOLDER_TYPE_MAX);
	
	if(folder->type)
	{
		//itoa not standard/supported on linux?	
		snprintf(type, sizeof(type)/sizeof(type[0]), "%d", folder->type);
	}
	
	gchar *strings[4] = {folder->parent_id, folder->folder_id, folder->display_name, type};

	*result = strconcatwithseparator(strings, sizeof(strings)/sizeof(strings[0]), folder_separator);

	g_print("serialise result: \n");
	g_print("%s\n", *result);
	
	if(!*result)
	{
		ret = FALSE;
	}

	g_print("eas_folder_serialise--\n");	
	return (*result? TRUE : FALSE);
}


// populate the folder object from a null terminated string eg ",1,,,". 
gboolean 
eas_folder_deserialise(EasFolder* folder, const gchar *data)
{
	gboolean ret = TRUE;
	
	g_print("eas_folder_deserialise++\n");

	g_assert(folder);
	g_assert(data);
	
	gchar *from = (gchar*)data;
	gchar *type;
	
	// parent_id
	if(folder->parent_id != NULL)   //expect empty structure, but just in case
	{
		g_free(folder->parent_id);
	}
	folder->parent_id = get_next_field(&from, folder_separator);
	if(!folder->parent_id)
	{
		ret = FALSE;
		goto cleanup;
	}

	// server_id
	if(folder->folder_id != NULL)
	{
		g_free(folder->folder_id);
	}	
	folder->folder_id = get_next_field(&from, folder_separator);
	if(!folder->folder_id)
	{
		ret = FALSE;
		goto cleanup;
	}	
	
	// display_name
	if(folder->display_name != NULL)
	{
		g_free(folder->display_name);
	}	
	folder->display_name = get_next_field(&from, folder_separator);
	if(!folder->display_name)
	{
		ret = FALSE;
		goto cleanup;
	}	
	
	//type
	type = get_next_field(&from, folder_separator);
	if(!type)
	{
		ret = FALSE;
		goto cleanup;
	}
	if(strlen(type))
	{
		folder->type = atoi(type);
		g_free(type);
	}

cleanup:
	if(!ret)
		{
			g_free(folder->parent_id);
			folder->parent_id = NULL;
			g_free(folder->folder_id);
			folder->folder_id = NULL;
			g_free(folder->display_name);
			folder->display_name = NULL;
		}

	// TODO remove debug prints
	g_print("deserialise result:\n");
	g_print("parent_id = %s\n", folder->parent_id);
	g_print("folder_id = %s\n", folder->folder_id);
	g_print("display name = %s\n", folder->display_name);
	g_print("type = %d\n", folder->type);
	
	g_print("eas_folder_deserialise--\n");	

	return ret;
}

