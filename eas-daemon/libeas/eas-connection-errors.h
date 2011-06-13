#ifndef EAS_ERRORS_H
#define EAS_ERRORS_H

#include <glib.h>

G_BEGIN_DECLS

GQuark eas_connection_error_quark (void);

#define EAS_CONNECTION_ERROR (eas_connection_error_quark ())

enum _EasConnectionError{
    EAS_CONNECTION_ERROR_INVALIDCONTENT=101,
    EAS_CONNECTION_ERROR_INVALIDWBXML=102,
    EAS_CONNECTION_ERROR_INVALIDXML=103,
    EAS_CONNECTION_ERROR_INVALIDDATETIME=104,
    EAS_CONNECTION_ERROR_INVALIDCOMBINATIONOFIDS=105,
    EAS_CONNECTION_ERROR_INVALIDIDS=106,
    EAS_CONNECTION_ERROR_INVALIDMIME=107,
    EAS_CONNECTION_ERROR_DEVICEIDMISSINGORINVALID=108,
    EAS_CONNECTION_ERROR_DEVICETYPEMISSINGORINVALID=109,
    EAS_CONNECTION_ERROR_SERVERERROR=110,
    EAS_CONNECTION_ERROR_SERVERERRORRETRYLATER=111,
    EAS_CONNECTION_ERROR_ACTIVEDIRECTORYACCESSDENIED=112,
    EAS_CONNECTION_ERROR_MAILBOXQUOTAEXCEEDED=113,
    EAS_CONNECTION_ERROR_MAILBOXSERVEROFFLINE=114,
    EAS_CONNECTION_ERROR_SENDQUOTAEXCEEDED=115,
    EAS_CONNECTION_ERROR_MESSAGERECIPIENTUNRESOLVED=116,
    EAS_CONNECTION_ERROR_MESSAGEREPLYNOTALLOWED=117,
    EAS_CONNECTION_ERROR_MESSAGEPREVIOUSLYSENT=118,
    EAS_CONNECTION_ERROR_MESSAGEHASNORECIPIENT=119,
    EAS_CONNECTION_ERROR_MAILSUBMISSIONFAILED=120,
    EAS_CONNECTION_ERROR_MESSAGEREPLYFAILED=121,
    EAS_CONNECTION_ERROR_ATTACHMENTISTOOLARGE=122,
    EAS_CONNECTION_ERROR_USERHASNOMAILBOX=123,
    EAS_CONNECTION_ERROR_USERCANNOTBEANONYMOUS=124,
    EAS_CONNECTION_ERROR_USERPRINCIPALCOULDNOTBEFOUND=125,
    EAS_CONNECTION_ERROR_USERDISABLEDFORSYNC=126,
    EAS_CONNECTION_ERROR_USERONNEWMAILBOXCANNOTSYNC=127,
    EAS_CONNECTION_ERROR_USERONLEGACYMAILBOXCANNOTSYNC=128,
    EAS_CONNECTION_ERROR_DEVICEISBLOCKEDFORTHISUSER=129,
    EAS_CONNECTION_ERROR_ACCESSDENIED=130,
    EAS_CONNECTION_ERROR_ACCOUNTDISABLED=131,
    EAS_CONNECTION_ERROR_SYNCSTATENOTFOUND=132,
    EAS_CONNECTION_ERROR_SYNCSTATELOCKED=133,
    EAS_CONNECTION_ERROR_SYNCSTATECORRUPT=134,
    EAS_CONNECTION_ERROR_SYNCSTATEALREADYEXISTS=135,
    EAS_CONNECTION_ERROR_SYNCSTATEVERSIONINVALID=136,
    EAS_CONNECTION_ERROR_COMMANDNOTSUPPORTED=137,
    EAS_CONNECTION_ERROR_VERSIONNOTSUPPORTED=138,
    EAS_CONNECTION_ERROR_DEVICENOTFULLYPROVISIONABLE=139,
    EAS_CONNECTION_ERROR_REMOTEWIPEREQUESTED=140,
    EAS_CONNECTION_ERROR_LEGACYDEVICEONSTRICTPOLICY=141,
    EAS_CONNECTION_ERROR_DEVICENOTPROVISIONED=142,
    EAS_CONNECTION_ERROR_POLICYREFRESH=143,
    EAS_CONNECTION_ERROR_INVALIDPOLICYKEY=144,
    EAS_CONNECTION_ERROR_EXTERNALLYMANAGEDDEVICESNOTALLOWED=145,
    EAS_CONNECTION_ERROR_NORECURRENCEINCALENDAR=146,
    EAS_CONNECTION_ERROR_UNEXPECTEDITEMCLASS=147,
    EAS_CONNECTION_ERROR_REMOTESERVERHASNOSSL=148,
    EAS_CONNECTION_ERROR_INVALIDSTOREDREQUEST=149,
    EAS_CONNECTION_ERROR_ITEMNOTFOUND=150,
    EAS_CONNECTION_ERROR_TOOMANYFOLDERS=151,
    EAS_CONNECTION_ERROR_NOFOLDERSFOUND=152,
    EAS_CONNECTION_ERROR_ITEMSLOSTAFTERMOVE=153,
    EAS_CONNECTION_ERROR_FAILUREINMOVEOPERATION=154,
    EAS_CONNECTION_ERROR_MOVECOMMANDDISALLOWEDFORNONPERSISTENTMOVEACTION=155,
    EAS_CONNECTION_ERROR_MOVECOMMANDINVALIDDESTINATIONFOLDER=156,
    EAS_CONNECTION_ERROR_AVAILABILITYTOOMANYRECIPIENTS=160,
    EAS_CONNECTION_ERROR_AVAILABILITYDLLIMITREACHED=161,
    EAS_CONNECTION_ERROR_AVAILABILITYTRANSIENTFAILURE=162,
    EAS_CONNECTION_ERROR_AVAILABILITYFAILURE=163,
    EAS_CONNECTION_ERROR_BODYPARTPREFERENCETYPENOTSUPPORTED=164,
    EAS_CONNECTION_ERROR_DEVICEINFORMATIONREQUIRED=165,
    EAS_CONNECTION_ERROR_INVALIDACCOUNTID=166,
    EAS_CONNECTION_ERROR_ACCOUNTSENDDISABLED=167,
    EAS_CONNECTION_ERROR_IRM_FEATUREDISABLED=168,
    EAS_CONNECTION_ERROR_IRM_TRANSIENTERROR=169,
    EAS_CONNECTION_ERROR_IRM_PERMANENTERROR=170,
    EAS_CONNECTION_ERROR_IRM_INVALIDTEMPLATEID=171,
    EAS_CONNECTION_ERROR_IRM_OPERATIONNOTPERMITTED=172,
    EAS_CONNECTION_ERROR_NOPICTURE=173,
    EAS_CONNECTION_ERROR_PICTURETOOLARGE=174,
    EAS_CONNECTION_ERROR_PICTURELIMITREACHED=175,
    EAS_CONNECTION_ERROR_BODYPART_CONVERSATIONTOOLARGE=176,
    EAS_CONNECTION_ERROR_MAXIMUMDEVICESREACHED=177,
    /* Below this point no longer direct translation of activesync errors */
	
    EAS_CONNECTION_ERROR_FAILED,
	EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
	EAS_CONNECTION_ERROR_BADARG,
	EAS_CONNECTION_ERROR_WBXMLERROR,
	EAS_CONNECTION_ERROR_SOUPERROR,
	EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,	// generic error for use by response parsers

	/*  Provision status errors  */
	EAS_CONNECTION_PROVISION_ERROR_PROTOCOLERROR,				
	EAS_CONNECTION_PROVISION_ERROR_GENERALSERVERERROR,			
	EAS_CONNECTION_PROVISION_ERROR_DEVICE_EXTERNALLY_MANAGED,	
	EAS_CONNECTION_PROVISION_ERROR_NOCLIENTPOLICYEXISTS,
	EAS_CONNECTION_PROVISION_ERROR_UNKNOWNPOLICYTYPE,
	EAS_CONNECTION_PROVISION_ERROR_CORRUPTSERVERPOLICYDATA,
	EAS_CONNECTION_PROVISION_ERROR_ACKINGWRONGPOLICYKEY,
	EAS_CONNECTION_PROVISION_ERROR_STATUSUNRECOGNIZED,

	/* Sync status errors  */
	EAS_CONNECTION_SYNC_ERROR_INVALIDSYNCKEY,	
	EAS_CONNECTION_SYNC_ERROR_PROTOCOLERROR,	   
	EAS_CONNECTION_SYNC_ERROR_SERVERERROR,	   
	EAS_CONNECTION_SYNC_ERROR_CONVERSIONERROR,
	EAS_CONNECTION_SYNC_ERROR_CONFLICTERROR,	   
	EAS_CONNECTION_SYNC_ERROR_OBJECTNOTFOUND,
	EAS_CONNECTION_SYNC_ERROR_MAILBOXFULL,
	EAS_CONNECTION_SYNC_ERROR_FOLDERHIERARCHYCHANGED,
	EAS_CONNECTION_SYNC_ERROR_REQUESTINCOMPLETE,
	EAS_CONNECTION_SYNC_ERROR_INVALIDWAITORHEARTBEAT,
	EAS_CONNECTION_SYNC_ERROR_INVALIDSYNCCOMMAND,
	EAS_CONNECTION_SYNC_ERROR_RETRY,
	EAS_CONNECTION_SYNC_ERROR_STATUSUNRECOGNIZED,	

	
} ;

typedef enum _EasConnectionError EasConnectionError;

/*
struct EasErrorMap {
	const gchar *error_id;
	gint error_code;
};

gint eas_get_error_code (const gchar *str);
*/

enum EasSyncStatus
{
	EAS_SYNC_STATUS_OK = 1,
	EAS_SYNC_STATUS_INVALIDSYNCKEY = 3,
	EAS_SYNC_STATUS_PROTOCOLERROR = 4,
	EAS_SYNC_STATUS_SERVERERROR = 5,
	EAS_SYNC_STATUS_CONVERSIONERROR = 6,
	EAS_SYNC_STATUS_CONFLICTERROR = 7,
	EAS_SYNC_STATUS_OBJECTNOTFOUND = 8,
	EAS_SYNC_STATUS_MAILBOXFULL = 9,
	EAS_SYNC_STATUS_FOLDERHIERARCHYCHANGED = 12,
	EAS_SYNC_STATUS_REQUESTINCOMPLETE = 13,
	EAS_SYNC_STATUS_INVALIDWAITORHEARTBEAT = 14,
	EAS_SYNC_STATUS_INVALIDSYNCCOMMAND = 15,
	EAS_SYNC_STATUS_RETRY = 16,
	EAS_SYNC_STATUS_LIMIT,   // no sync status spec'd above 16 currently
};

struct _EasError{
	EasConnectionError code;
	const gchar *message;
};

typedef struct _EasError EasError;

extern EasError status_error_map[];

G_END_DECLS

#endif
