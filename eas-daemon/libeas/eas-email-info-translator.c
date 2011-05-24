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

#include "eas-email-info-translator.h"
#include "../../libeasmail/src/eas-folder.h"
#include "../../libeasmail/src/eas-attachment.h"
#include "../../libeasmail/src/eas-email-info.h"


G_DEFINE_TYPE (EasEmailInfoTranslator, eas_email_info_translator, G_TYPE_OBJECT);

static void
eas_email_info_translator_init (EasEmailInfoTranslator *object)
{
	/* TODO: Add initialization code here */
}

static void
eas_email_info_translator_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (eas_email_info_translator_parent_class)->finalize (object);
}

static void
eas_email_info_translator_class_init (EasEmailInfoTranslatorClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eas_email_info_translator_finalize;
}


gchar * 
eas_add_email_appdata_parse_response (EasEmailInfoTranslator* self, xmlNode *node, gchar *server_id)
{
	gchar *result = NULL;
	
	if (!node) return;

	// Parse the email specific (applicationdata) part of a sync response
	// and generate the app-specific parts of the EasEmailInfo structure
    if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "ApplicationData")) 
	{
		EasEmailInfo *email_info = eas_email_info_new();
		xmlNode *n = node;
		GSList *headers = NULL;
		GSList *attachments = NULL;
		guint8  flags = 0;
		GSList *categories = NULL;

		for (n = n->children; n; n = n->next)
		{
			//To
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "email:To"))	//TODO - is it necessary to specify the namespace?
			{
				EasEmailHeader *header = g_malloc0(sizeof(EasEmailHeader));  
				header->name = g_strdup("To");
				header->value = (gchar *)xmlNodeGetContent(n); 
				headers = g_slist_append(headers, header);
				continue;
			}		
			//Cc		
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "email:Cc")) 
			{
				EasEmailHeader *header = g_malloc0(sizeof(EasEmailHeader));  
				header->name = g_strdup("Cc");
				header->value = (gchar *)xmlNodeGetContent(n);
				headers = g_slist_append(headers, header);
				continue;
			}			
			//From
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "email:From")) 
			{
				EasEmailHeader *header = g_malloc0(sizeof(EasEmailHeader));  
				header->name = g_strdup("From");
				header->value = (gchar *)xmlNodeGetContent(n);
				headers = g_slist_append(headers, header);
				continue;
			}				
			//Subject
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "email:Subject")) 
			{
				EasEmailHeader *header = g_malloc0(sizeof(EasEmailHeader));  
				header->name = g_strdup("Subject");
				header->value = (gchar *)xmlNodeGetContent(n);
				headers = g_slist_append(headers, header);
				continue;
			}			
			
			//Reply-To
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "email:ReplyTo")) 
			{
				EasEmailHeader *header = g_malloc0(sizeof(EasEmailHeader));  
				header->name = g_strdup("Reply-To");	//MIME equivalent
				header->value = (gchar *)xmlNodeGetContent(n);
				headers = g_slist_append(headers, header);
				continue;
			}				
			
			//Received
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "email:DateReceived")) 
			{
				EasEmailHeader *header = g_malloc0(sizeof(EasEmailHeader));  
				header->name = g_strdup("Received");	// MIME equivalent
				header->value = (gchar *)xmlNodeGetContent(n);
				headers = g_slist_append(headers, header);
				continue;
			}			
				
			//DisplayTo	 - is there an equivalent standard email header?
			
			//Importance
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "email:Importance")) 
			{
				EasEmailHeader *header = g_malloc0(sizeof(EasEmailHeader));  
				header->name = g_strdup("Importance");
				header->value = (gchar *)xmlNodeGetContent(n);
				headers = g_slist_append(headers, header);
				continue;
			}			
			//Read  
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "email:Read")) 
			{
				if((gboolean)xmlNodeGetContent(n))
				{
					flags |= EAS_EMAIL_READ;
				}
				continue;
			}	
			// TODO which if any of these other headers are standard email headers?	
			//ThreadTopic   TODO is this where we get the answered/forwarded 'flags' from?			
			//MessageClass			-   ?
			//MeetingRequest stuff  -   ignoring, not MIME header
			//InternetCPID			-   ignoring, EAS specific
			//Task 'Flag' stuff		-   ignoring, not MIME header
			//ContentClass			-   ?

			//Attachments
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "airsyncbase:Attachments")) 
			{ 
				xmlNode *s = n;
				for (s = s->children; s; s = s->next)
				{				
					if (s->type == XML_ELEMENT_NODE && !strcmp((char *)s->name, "airsyncbase:Attachment"))					
					{
						EasAttachment *attachment = g_malloc0(sizeof(EasAttachment)); 
						
						//DisplayName
						if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "airsyncbase:DisplayName")) 						
						{
							attachment->display_name = (gchar *)xmlNodeGetContent(n);
						}
						//EstimatedDataSize
						if (n->type == XML_ELEMENT_NODE && !strcmp((guint)n->name, "airsyncbase:EstimatedDataSize")) 						
						{
							attachment->estimated_size = (guint)xmlNodeGetContent(n);
						}
						//FileReference
						if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "airsyncbase:FileReference")) 						
						{
							attachment->file_reference = (gchar *)xmlNodeGetContent(n);
						}						
						//Method			- not storing
						attachments = g_slist_append(attachments, (char *)xmlNodeGetContent(s));
					}
				}
				continue;
			}
			
			//Categories
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "email:Categories")) 
			{
				xmlNode *s = n;
				for (s = s->children; s; s = s->next)
				{				
					if (s->type == XML_ELEMENT_NODE && !strcmp((char *)s->name, "Category"))					
					{
						categories = g_slist_append(categories, (char *)xmlNodeGetContent(s));
					}
				}
				continue;
			}				
		} // end for 
		email_info->server_id = server_id;
		email_info->headers = headers;
		email_info->attachments = attachments;
		email_info->categories = categories;
		email_info->flags = flags;

		// serialise the emailinfo
		if(!eas_email_info_serialise(email_info, &result))
		{
			g_warning("Failed to serialise email info\n");
		}
		
		g_object_unref(email_info);
	}
	else
	{
		g_error("Failed! Expected ApplicationData node at root\n");
	}

	return result;
}

gchar * 
eas_update_email_appdata_parse_response (EasEmailInfoTranslator* self, xmlNode *node, gchar *server_id)
{
	gchar *result = NULL;
	
	if (!node) return;
	
    if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "ApplicationData")) 
	{
		EasEmailInfo *email_info = eas_email_info_new();
		GSList *categories = NULL;
		guint flags = 0;
		xmlNode *n = node;
		
		for (n = n->children; n; n = n->next)
		{
			// TODO - pull out other potentially updated data (flags/categories):
			//Read  
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "email:Read")) 
			{
				if((gboolean)xmlNodeGetContent(n))
				{
					flags |= EAS_EMAIL_READ;
				}
				continue;
			}			
			//Categories 
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "email:Categories")) 
			{
				xmlNode *s = n;
				for (s = s->children; s; s = s->next)
				{				
					if (s->type == XML_ELEMENT_NODE && !strcmp((char *)s->name, "Category"))					
					{
						categories = g_slist_append(categories, (char *)xmlNodeGetContent(s));
					}
				}
				continue;
			}		
		} // end for
		
		email_info->server_id = server_id;
		email_info->categories = categories;
		email_info->flags = flags;		
		
		g_object_unref(email_info);
	}
	else
	{
		g_error("Failed! Expected ApplicationData node at root\n");
	}
	return result;
}


gchar * 
eas_delete_email_appdata_parse_response (EasEmailInfoTranslator* self, xmlNode *node, gchar *server_id)
{
	gchar *result = NULL;
	
	if (!node) return;
	
    if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "ApplicationData")) 
	{
		EasEmailInfo *email_info = eas_email_info_new();

		email_info->server_id = server_id;
		// no other data supplied for deleted email, done
		
		if(!eas_email_info_serialise(email_info, &result))
		{
			g_warning("Failed to serialise email info\n");
		}
		
		g_object_unref(email_info);
	}
	else
	{
		g_error("Failed! Expected ApplicationData node at root\n");
	}

	return result;
}
