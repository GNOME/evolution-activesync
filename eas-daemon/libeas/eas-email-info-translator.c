/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
	
 */

#include "eas-email-info-translator.h"
#include "../../libeasmail/src/eas-folder.h"
#include "../../libeasmail/src/eas-attachment.h"
#include "../../libeasmail/src/eas-email-info.h"


gchar * 
eas_add_email_appdata_parse_response (xmlNode *node, gchar *server_id)
{
	g_debug("eas_add_email_appdata_parse_response++");
	gchar *result = NULL;
	
	if (!node) return;

	// Parse the email specific (applicationdata) part of a sync response
	// and generate the app-specific parts of the EasEmailInfo structure
    if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "ApplicationData")) 
	{
		g_debug("found ApplicationData root");
		EasEmailInfo *email_info = eas_email_info_new();
		xmlNode *n = node;
		GSList *headers = NULL;
		GSList *attachments = NULL;
		guint8  flags = 0;
		GSList *categories = NULL;

		for (n = n->children; n; n = n->next)
		{
			//To
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "To"))	
			{
				EasEmailHeader *header = g_malloc0(sizeof(EasEmailHeader));  
				header->name = g_strdup("To");
				header->value = (gchar *)xmlNodeGetContent(n);  //takes ownership of the memory
				headers = g_slist_append(headers, header);
				continue;
			}		
			//Cc		
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "Cc")) 
			{
				EasEmailHeader *header = g_malloc0(sizeof(EasEmailHeader));  
				header->name = g_strdup("Cc");
				header->value = (gchar *)xmlNodeGetContent(n);
				headers = g_slist_append(headers, header);
				continue;
			}			
			//From
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "From")) 
			{
				EasEmailHeader *header = g_malloc0(sizeof(EasEmailHeader));  
				header->name = g_strdup("From");
				header->value = (gchar *)xmlNodeGetContent(n);
				headers = g_slist_append(headers, header);
				continue;
			}				
			//Subject
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "Subject")) 
			{
				EasEmailHeader *header = g_malloc0(sizeof(EasEmailHeader));  
				header->name = g_strdup("Subject");
				header->value = (gchar *)xmlNodeGetContent(n);
				headers = g_slist_append(headers, header);
				continue;
			}			
			
			//Reply-To
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "ReplyTo")) 
			{
				EasEmailHeader *header = g_malloc0(sizeof(EasEmailHeader));  
				header->name = g_strdup("Reply-To");	//MIME equivalent
				header->value = (gchar *)xmlNodeGetContent(n);
				headers = g_slist_append(headers, header);
				continue;
			}				
			
			//Received
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "DateReceived")) 
			{
				EasEmailHeader *header = g_malloc0(sizeof(EasEmailHeader));  
				header->name = g_strdup("Received");	// MIME equivalent
				header->value = (gchar *)xmlNodeGetContent(n);
				headers = g_slist_append(headers, header);
				continue;
			}			
				
			//DisplayTo	 - is there an equivalent standard email header?
			
			//Importance
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "Importance")) 
			{
				EasEmailHeader *header = g_malloc0(sizeof(EasEmailHeader));  
				header->name = g_strdup("Importance");
				header->value = (gchar *)xmlNodeGetContent(n);
				headers = g_slist_append(headers, header);
				continue;
			}			
			//Read  
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "Read")) 
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
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "Attachments")) 
			{ 
				g_debug("found attachments");
				xmlNode *s = n;
				for (s = s->children; s; s = s->next)
				{				
					if (s->type == XML_ELEMENT_NODE && !strcmp((gchar *)s->name, "Attachment"))					
					{
						g_debug("found attachment");
						
						EasAttachment *attachment = g_malloc0(sizeof(EasAttachment)); 

						xmlNode *t = s;
						for (t = t->children; t; t = t->next)
						{						
							//DisplayName
							if (t->type == XML_ELEMENT_NODE && !strcmp((gchar *)t->name, "DisplayName")) 						
							{
								attachment->display_name = (gchar *)xmlNodeGetContent(t);
								g_debug("attachment name = %s", attachment->display_name);							
							}
							//EstimatedDataSize
							if (t->type == XML_ELEMENT_NODE && !strcmp((guint)t->name, "EstimatedDataSize")) 						
							{
								attachment->estimated_size = (guint)xmlNodeGetContent(t);
								g_debug("attachment size = %d", attachment->estimated_size);							
							}
							//FileReference
							if (t->type == XML_ELEMENT_NODE && !strcmp((gchar *)t->name, "FileReference")) 						
							{
								attachment->file_reference = (gchar *)xmlNodeGetContent(t);
								g_debug("file reference = %s", attachment->file_reference);
							}						
							//Method			- not storing
						}
						attachments = g_slist_append(attachments, attachment);						
					}
				}
				continue;
			}
			
			//Categories
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "Categories")) 
			{
				g_debug("found categories");
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
			g_warning("Failed to serialise email info");
		}

		g_object_unref(email_info);   
	}
	else
	{
		g_error("Failed! Expected ApplicationData node at root");
	}

	g_debug("eas_add_email_appdata_parse_response--");	
	return result;
}

gchar * 
eas_update_email_appdata_parse_response (xmlNode *node, gchar *server_id)
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
			// TODO - figure out if/where other flags are stored (eg replied to)
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
		g_error("Failed! Expected ApplicationData node at root");
	}
	return result;
}


gchar * 
eas_delete_email_appdata_parse_response (xmlNode *node, gchar *server_id)
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
			g_warning("Failed to serialise email info");
		}
		
		g_object_unref(email_info);
	}
	else
	{
		g_error("Failed! Expected ApplicationData node at root");
	}

	return result;
}

// translate the other way: take the emailinfo object and convert to ApplicationData node
// takes a flattened email_info object and passes back an xmlnode (with application data at root) and a serverid
void
eas_update_email_appdata_create_request_node (gchar *flat_email_info, xmlNode **node, gchar **server_id)
{
	// TODO (subfeature 17e)
}
