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

#include "eas-send-email-msg.h"

G_DEFINE_TYPE (EasSendEmailMsg, eas_send_email_msg, G_TYPE_OBJECT);

#define EAS_SEND_EMAIL_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SEND_EMAIL_MSG, EasSendEmailMsgPrivate))

struct _EasSendEmailMsgPrivate
{
	//TODO
	/*
	guint64 accountID;
	gchar* clientid; 
	gchar* mime_file;	
	 */
};

static void
eas_send_email_msg_init (EasSendEmailMsg *object)
{
	/* TODO: Add initialization code here */
	g_debug("eas_send_email_msg_init++");

	EasSendEmailMsgPrivate *priv;

	object->priv = priv = EAS_SEND_EMAIL_MSG_PRIVATE(object);	

	g_debug("eas_send_email_msg_init--");
}

static void
eas_send_email_msg_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	EasSendEmailMsg *msg = (EasSendEmailMsg *)object;
	
	EasSendEmailMsgPrivate *priv = msg->priv;	
	g_free (priv);
	msg->priv = NULL;
	G_OBJECT_CLASS (eas_send_email_msg_parent_class)->finalize (object);
}

static void
eas_send_email_msg_class_init (EasSendEmailMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eas_send_email_msg_finalize;
}

