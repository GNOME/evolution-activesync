/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*

 */

#include "eas-email-info-translator.h"
#include "../../libeasmail/src/eas-folder.h"
#include "../../libeasmail/src/eas-attachment.h"
#include "../../libeasmail/src/eas-email-info.h"
#include <libxml/tree.h>


gchar *
eas_add_email_appdata_parse_response (xmlNode *node, gchar *server_id)
{
    gchar *result = NULL;
    g_debug ("eas_add_email_appdata_parse_response++");

    if (!node) return NULL;

    // Parse the email specific (applicationdata) part of a sync response
    // and generate the app-specific parts of the EasEmailInfo structure
    if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "ApplicationData"))
    {
        EasEmailInfo *email_info = eas_email_info_new();
        xmlNode *n = node;
        GSList *headers = NULL;
        GSList *attachments = NULL;
        guint8  flags = 0;
        GSList *categories = NULL;

        g_debug ("found ApplicationData root");

        for (n = n->children; n; n = n->next)
        {
            //To
            if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "To"))
            {
                EasEmailHeader *header = g_malloc0 (sizeof (EasEmailHeader));
                header->name = g_strdup ("To");
                header->value = (gchar *) xmlNodeGetContent (n);  //takes ownership of the memory
                headers = g_slist_append (headers, header);
            }
            //Cc
            else if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Cc"))
            {
                EasEmailHeader *header = g_malloc0 (sizeof (EasEmailHeader));
                header->name = g_strdup ("Cc");
                header->value = (gchar *) xmlNodeGetContent (n);
                headers = g_slist_append (headers, header);
            }
            //From
            else if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "From"))
            {
                EasEmailHeader *header = g_malloc0 (sizeof (EasEmailHeader));
                header->name = g_strdup ("From");
                header->value = (gchar *) xmlNodeGetContent (n);
                headers = g_slist_append (headers, header);
            }
            //Subject
            else if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Subject"))
            {
                EasEmailHeader *header = g_malloc0 (sizeof (EasEmailHeader));
                header->name = g_strdup ("Subject");
                header->value = (gchar *) xmlNodeGetContent (n);
                headers = g_slist_append (headers, header);
            }

            //Reply-To
            else if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "ReplyTo"))
            {
                EasEmailHeader *header = g_malloc0 (sizeof (EasEmailHeader));
                header->name = g_strdup ("Reply-To");   //MIME equivalent
                header->value = (gchar *) xmlNodeGetContent (n);
                headers = g_slist_append (headers, header);
            }
            //Received TODO date-received is NOT a standard MIME header and should be put into a separate field in email info
            else if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "DateReceived"))
            {
                EasEmailHeader *header = g_malloc0 (sizeof (EasEmailHeader));
                header->name = g_strdup ("Received");   // MIME equivalent
                header->value = (gchar *) xmlNodeGetContent (n);
                headers = g_slist_append (headers, header);
            }
            //DisplayTo  - is there an equivalent standard email header?
            //Importance
            else if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Importance"))
            {
                EasEmailHeader *header = g_malloc0 (sizeof (EasEmailHeader));
                header->name = g_strdup ("Importance");
                header->value = (gchar *) xmlNodeGetContent (n);
                headers = g_slist_append (headers, header);
            }
            //Read
            else if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Read"))
            {
                gchar *tmp = (gchar*) xmlNodeGetContent (n);
                if (g_strcmp0 (tmp, "0")) // not 0, therefore read
                {
                    flags |= EAS_EMAIL_READ;
                }
                g_free (tmp);
            }
            // TODO which if any of these other headers are standard email headers?
            //ThreadTopic
            //MessageClass          -   ?
            //MeetingRequest stuff  -   ignoring, not MIME header
            //InternetCPID          -   ignoring, EAS specific
            //Task 'Flag' stuff     -   ignoring, not MIME header
            //ContentClass          -   ?

            //Attachments
            else if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Attachments"))
            {
                xmlNode *s = n;
                g_debug ("found attachments");
                for (s = s->children; s; s = s->next)
                {
                    if (s->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar *) s->name, "Attachment"))
                    {
                        EasAttachment *attachment = eas_attachment_new();
                        xmlNode *t = s;
                        g_debug ("found attachment");
                        for (t = t->children; t; t = t->next)
                        {
                            //DisplayName
                            if (t->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar *) t->name, "DisplayName"))
                            {
                                attachment->display_name = (gchar *) xmlNodeGetContent (t);
                                g_debug ("attachment name = %s", attachment->display_name);
                            }
                            //EstimatedDataSize
                            if (t->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar*) t->name, "EstimatedDataSize"))
                            {
                                attachment->estimated_size = (guint) xmlNodeGetContent (t);
                                g_debug ("attachment size = %d", attachment->estimated_size);
                            }
                            //FileReference
                            if (t->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar *) t->name, "FileReference"))
                            {
                                attachment->file_reference = (gchar *) xmlNodeGetContent (t);
                                g_debug ("file reference = %s", attachment->file_reference);
                            }
                            //Method            - not storing
                        }
                        attachments = g_slist_append (attachments, attachment);
                    }
                }
            }

            //Categories
            else if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Categories"))
            {
                xmlNode *s = n;
                g_debug ("found categories");
                for (s = s->children; s; s = s->next)
                {
                    if (s->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) s->name, "Category"))
                    {
                        categories = g_slist_append (categories, (char *) xmlNodeGetContent (s));
                    }
                }
            }
        } // end for

        email_info->server_id = server_id;
        email_info->headers = headers;
        email_info->attachments = attachments;
        email_info->categories = categories;
        email_info->flags = flags;

        // serialise the emailinfo
        if (!eas_email_info_serialise (email_info, &result))
        {
            g_warning ("Failed to serialise email info");
        }

        g_object_unref (email_info);
    }
    else
    {
        g_error ("Failed! Expected ApplicationData node at root");
    }

    g_debug ("eas_add_email_appdata_parse_response--");
    return result;
}

gchar *
eas_update_email_appdata_parse_response (xmlNode *node, gchar *server_id)
{
    gchar *result = NULL;
    g_debug ("eas_update_email_appdata_parse_response++");

    if (!node) return NULL;

    if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "ApplicationData"))
    {
        EasEmailInfo *email_info = eas_email_info_new();
        GSList *categories = NULL;
        guint flags = 0;
        xmlNode *n = node;

        g_debug ("found ApplicationData root");

        for (n = n->children; n; n = n->next)
        {
            // TODO - figure out if/where other flags are stored (eg replied to/forwarded in ConversationIndex?)
            //Read
            if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Read"))
            {
                gchar* tmp = (gchar*) xmlNodeGetContent (n);
                g_debug ("found read node");
                if (g_strcmp0 (tmp, "0")) // not 0, therefore read
                {
                    flags |= EAS_EMAIL_READ;
                }
                g_free (tmp);
                continue;
            }
            //Categories
            if (n->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) n->name, "Categories"))
            {
                xmlNode *s = n;
                for (s = s->children; s; s = s->next)
                {
                    if (s->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) s->name, "Category"))
                    {
                        categories = g_slist_append (categories, (char *) xmlNodeGetContent (s));
                    }
                }
                continue;
            }
        } // end for

        email_info->server_id = server_id;
        email_info->categories = categories;
        email_info->flags = flags;

        // serialise the emailinfo
        if (!eas_email_info_serialise (email_info, &result))
        {
            g_warning ("Failed to serialise email info");
        }

        g_object_unref (email_info);
    }
    else
    {
        g_error ("Failed! Expected ApplicationData node at root");
    }

    g_debug ("eas_update_email_appdata_parse_response--");
    return result;
}


gchar *
eas_delete_email_appdata_parse_response (xmlNode *node, gchar *server_id)
{
    gchar *result = NULL;

    if (!node) return NULL;

    if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "ApplicationData"))
    {
        EasEmailInfo *email_info = eas_email_info_new();

        email_info->server_id = server_id;
        // no other data supplied for deleted email, done

        if (!eas_email_info_serialise (email_info, &result))
        {
            g_warning ("Failed to serialise email info");
        }

        g_object_unref (email_info);
    }
    else
    {
        g_error ("Failed! Expected ApplicationData node at root");
    }

    return result;
}

static gboolean
create_node_from_categorylist (xmlNode *app_data, const GSList* categories)
{
    gboolean ret = TRUE;

    xmlNode *categories_node, *leaf = NULL;
    if (categories)
    {
        g_debug ("Creating Categories node");
        // Create the categories collection node
        // TODO - can namespace name (email) be included in name like this rather than as a xmlNsPtr param?
        categories_node = xmlNewChild (app_data, NULL, (xmlChar *) "email:Categories", NULL);
    }

    while (categories != NULL)
    {
        g_debug ("Creating Categorys node for %s", (gchar *) categories->data);
        leaf = xmlNewTextChild (categories_node, NULL, (xmlChar *) "email:Category", (xmlChar*) categories->data);
        if (!leaf)
        {
            ret = FALSE;
        }
        categories = categories->next;
    }

    return ret;
}


// translate the other way: take the emailinfo object and populate the ApplicationData node
gboolean
eas_email_info_translator_build_update_request (xmlDoc *doc, xmlNode *app_data, const EasEmailInfo *email_info)
{
    gboolean ret = FALSE;
    g_debug ("eas_email_info_translator_parse_request++");

    if (! (doc &&
            app_data &&
            email_info &&
            (app_data->type == XML_ELEMENT_NODE) &&
            (g_strcmp0 ( (char*) (app_data->name), "ApplicationData") == 0)))
    {
        g_debug ("invalid input");
    }
    else
    {
        // Note that the only fields it's valid to update are flags and categories!
        xmlNode *leaf;

        // flags
        if (email_info->flags & EAS_EMAIL_READ)
        {
            g_debug ("setting Read to 1");
            leaf = xmlNewChild (app_data, NULL, (xmlChar *) "Read", (xmlChar*) "1");
        }
        else
        {
            g_debug ("setting Read to 0");
            leaf = xmlNewChild (app_data, NULL, (xmlChar *) "Read", (xmlChar*) "0");
        }

        if (ret)
        {
            //categories
            ret = create_node_from_categorylist (app_data, email_info->categories);
        }
    }

    // DEBUG output TODO make configurable or comment out
    {
        xmlChar* dump_buffer;
        int dump_buffer_size;
        xmlIndentTreeOutput = 1;
        xmlDocDumpFormatMemory (doc, &dump_buffer, &dump_buffer_size, 1);
        g_debug ("XML DOCUMENT DUMPED:\n%s", dump_buffer);
        xmlFree (dump_buffer);
    }

    g_debug ("eas_email_info_translator_parse_request--");
    return ret;
}
