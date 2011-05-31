/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * intelgit
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_UPDATE_CALENDAR_REQ_H_
#define _EAS_UPDATE_CALENDAR_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_UPDATE_CALENDAR_REQ            (eas_update_calendar_req_get_type ())
#define EAS_UPDATE_CALENDAR_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_UPDATE_CALENDAR_REQ EasUpdateCalendarReq))
#define EAS_UPDATE_CALENDAR_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_UPDATE_CALENDAR_REQ, EasUpdateCalendarReqClass))
#define EAS_IS_UPDATE_CALENDAR_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_UPDATE_CALENDAR_REQ))
#define EAS_IS_UPDATE_CALENDAR_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_UPDATE_CALENDAR_REQ))
#define EAS_UPDATE_CALENDAR_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_UPDATE_CALENDAR_REQ, EasUpdateCalendarReqClass))

typedef struct _EasUpdateCalendarReqClass EasUpdateCalendarReqClass;
typedef struct _EasUpdateCalendarReq EasUpdateCalendarReq;
typedef struct _EasUpdateCalendarReqPrivate EasUpdateCalendarReqPrivate;

struct _EasUpdateCalendarReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasUpdateCalendarReq
{
	EasRequestBase parent_instance;
	
	EasUpdateCalendarReqPrivate * priv;
};

GType eas_update_calendar_req_get_type (void) G_GNUC_CONST;

// C'tor
EasUpdateCalendarReq *eas_update_calendar_req_new(guint64 account_id, const gchar *sync_key, const gchar *folder_id, const GSList *serialised_calendar, EFlag *flag);

// start async request
void eas_update_calendar_req_Activate(EasUpdateCalendarReq *self);

// async request completed
void eas_update_calendar_req_MessageComplete(EasUpdateCalendarReq *self, xmlDoc* doc, GError** error);

// results returned to client
void eas_update_calendar_req_ActivateFinish (EasUpdateCalendarReq* self, gchar** ret_sync_key, GError **error);

G_END_DECLS

#endif /* _EAS_UPDATE_CALENDAR_REQUEST_H_ */

