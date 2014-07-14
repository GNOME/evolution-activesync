#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>
#include <glib-object.h>

#include "../eas-daemon/libeas/eas-request-base.h"
#include "../eas-daemon/libeas/eas-add-item-req.h"
#include "../eas-daemon/libeas/eas-connection.h"
#include "../eas-daemon/libeas/eas-delete-req.h"
#include "../eas-daemon/libeas/eas-get-email-attachment-msg.h"
#include "../eas-daemon/libeas/eas-get-email-attachment-req.h"
#include "../eas-daemon/libeas/eas-get-email-body-msg.h"
#include "../eas-daemon/libeas/eas-get-email-body-req.h"
#include "../eas-daemon/libeas/eas-move-email-msg.h"
#include "../eas-daemon/libeas/eas-move-email-req.h"
#include "../eas-daemon/libeas/eas-msg-base.h"
#include "../eas-daemon/libeas/eas-ping-msg.h"
#include "../eas-daemon/libeas/eas-ping-req.h"
#include "../eas-daemon/libeas/eas-provision-msg.h"
#include "../eas-daemon/libeas/eas-provision-req.h"
#include "../eas-daemon/libeas/eas-request-base.h"
#include "../eas-daemon/libeas/eas-send-email-msg.h"
#include "../eas-daemon/libeas/eas-send-email-req.h"
#include "../eas-daemon/libeas/eas-sync-folder-hierarchy-req.h"
#include "../eas-daemon/libeas/eas-sync-folder-msg.h"
#include "../eas-daemon/libeas/eas-sync-msg.h"
#include "../eas-daemon/libeas/eas-sync-req.h"
#include "../eas-daemon/libeas/eas-update-email-req.h"
#include "../eas-daemon/libeas/eas-update-item-req.h"
#include "../eas-daemon/libeas/eas-get-item-estimate-req.h"
#include "../eas-daemon/libeas/eas-get-item-estimate-msg.h"
#include "../eas-daemon/src/eas-mail.h"
#include "../eas-daemon/src/eas-sync.h"
#include "../eas-daemon/src/eas-test.h"
#include "../libeasaccount/src/eas-account-list.h"
#include "../libeasaccount/src/eas-account.h"
#include "../libeasmail/src/eas-attachment.h"
#include "../libeasmail/src/eas-email-info.h"
#include "../libeasmail/src/eas-folder.h"
#include "../libeasmail/src/libeasmail.h"
#include "../libeassync/src/eas-item-info.h"
#include "../libeassync/src/libeassync.h"
#include "../libeastest/src/libeastest.h"
#include "../eas-daemon/libeas/eas-2way-sync-req.h"
Suite* eas_gobjectunittest_suite (void);
START_TEST (test_add_item_req_obj)
{
	EasAddItemReq* self_eas_add_item_req;
	EasConnection* self_eas_connection;
	EasDeleteReq* self_eas_delete_req;
	EasGetEmailAttachmentMsg* self_eas_get_email_attachment_msg;
	EasGetEmailAttachmentReq* self_eas_get_email_attachment_req;
	EasGetEmailBodyMsg* self_eas_get_email_body_msg;
	EasGetEmailBodyReq* self_eas_get_email_body_req;
	EasMoveEmailMsg* self_eas_move_email_msg;
	EasMoveEmailReq* self_eas_move_email_req;
	EasMsgBase* self_eas_msg_base;
	EasPingMsg* self_eas_ping_msg;
	EasPingReq* self_eas_ping_req;
	EasProvisionMsg* self_eas_provision_msg;
	EasProvisionReq* self_eas_provision_req;
	EasRequestBase* self_eas_request_base;
	EasSendEmailMsg* self_eas_send_email_msg;
	EasSendEmailReq* self_eas_send_email_req;
	EasSyncFolderHierarchyReq* self_eas_sync_folder_hierarchy_req;
	EasSyncMsg* self_eas_sync_msg;
	EasSyncFolderMsg* self_eas_sync_folder_msg;
	EasSyncReq* self_eas_sync_req;
	EasUpdateEmailReq* self_eas_update_email_req;
	EasUpdateItemReq* self_eas_update_item_req;
	EasMail* self_eas_mail;
	EasSyncHandler* self_eas_sync_handler;
	EasTestHandler* self_eas_test_handler;
	EasSyncHandler* self_eas_item_info;
	EasSync* self_eas_sync;
	EasEmailHandler* self_eas_email_handler;
	EasTest* self_eas_test;
	EasAccountList* self_eas_account_list;
	EasAccount* self_eas_account;
	EasGetItemEstimateReq *self_eas_get_item_estimate_req;
	EasGetItemEstimateMsg *self_eas_get_item_estimate_msg;	
	Eas2WaySyncReq* self_eas_2way_sync_req;
	EasAttachment* self_eas_attachment;
	EasEmailInfo* self_eas_email_info;
	EasFolder* self_eas_folder;

	
    // initise G type library to allow use of G type objects which provide
    // a higher level of programming syntax including the psuedo object
    // oriented approach supported by G types
#if !GLIB_CHECK_VERSION(2,36,0)
	g_type_init();
#endif

	g_debug("gobject: EAS_TYPE_ADD_ITEM_REQ ++");
	self_eas_add_item_req = g_object_new (EAS_TYPE_ADD_ITEM_REQ, NULL);
	g_object_ref(self_eas_add_item_req);
	g_object_unref(self_eas_add_item_req);
	g_object_unref(self_eas_add_item_req);	
	g_debug("EAS_TYPE_ADD_ITEM_REQ --");

	g_debug("EAS_TYPE_CONNECTION ++");
	self_eas_connection = g_object_new (EAS_TYPE_CONNECTION, NULL);
	g_object_ref(self_eas_connection);
	g_object_unref(self_eas_connection);
	g_object_unref(self_eas_connection);	
	g_debug("EAS_TYPE_CONNECTION --");

	g_debug("EAS_TYPE_DELETE_REQ ++");
	self_eas_delete_req = g_object_new (EAS_TYPE_DELETE_REQ, NULL);
	g_object_ref(self_eas_delete_req);
	g_object_unref(self_eas_delete_req);
	g_object_unref(self_eas_delete_req);	
	g_debug("EAS_TYPE_DELETE_REQ --");

	g_debug("EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG ++");
	self_eas_get_email_attachment_msg = g_object_new (EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG, NULL);
	g_object_ref(self_eas_get_email_attachment_msg);
	g_object_unref(self_eas_get_email_attachment_msg);
	g_object_unref(self_eas_get_email_attachment_msg);	
	g_debug("EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG --");

	g_debug("EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ ++");
	self_eas_get_email_attachment_req = g_object_new (EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ, NULL);
	g_object_ref(self_eas_get_email_attachment_req);
	g_object_unref(self_eas_get_email_attachment_req);
	g_object_unref(self_eas_get_email_attachment_req);	
	g_debug("EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ --");

	g_debug("EAS_TYPE_GET_EMAIL_BODY_MSG ++");
	self_eas_get_email_body_msg = g_object_new (EAS_TYPE_GET_EMAIL_BODY_MSG, NULL);
	g_object_ref(self_eas_get_email_body_msg);
	g_object_unref(self_eas_get_email_body_msg);
	g_object_unref(self_eas_get_email_body_msg);	
	g_debug("EAS_TYPE_GET_EMAIL_BODY_MSG --");

	g_debug("EAS_TYPE_GET_EMAIL_BODY_REQ ++");
	self_eas_get_email_body_req = g_object_new (EAS_TYPE_GET_EMAIL_BODY_REQ, NULL);
	g_object_ref(self_eas_get_email_body_req);
	g_object_unref(self_eas_get_email_body_req);
	g_object_unref(self_eas_get_email_body_req);	
	g_debug("EAS_TYPE_GET_EMAIL_BODY_REQ --");

	g_debug("EAS_TYPE_MOVE_EMAIL_MSG ++");
	self_eas_move_email_msg = g_object_new (EAS_TYPE_MOVE_EMAIL_MSG, NULL);
	g_object_ref(self_eas_move_email_msg);
	g_object_unref(self_eas_move_email_msg);
	g_object_unref(self_eas_move_email_msg);	
	g_debug("EAS_TYPE_MOVE_EMAIL_MSG --");

	g_debug("EAS_TYPE_MOVE_EMAIL_REQ ++");
	self_eas_move_email_req = g_object_new (EAS_TYPE_MOVE_EMAIL_REQ, NULL);
	g_object_ref(self_eas_move_email_req);
	g_object_unref(self_eas_move_email_req);
	g_object_unref(self_eas_move_email_req);	
	g_debug("EAS_TYPE_MOVE_EMAIL_REQ --");

	g_debug("EAS_TYPE_MSG_BASE ++");
	self_eas_msg_base = g_object_new (EAS_TYPE_MSG_BASE, NULL);
	g_object_ref(self_eas_msg_base);
	g_object_unref(self_eas_msg_base);
	g_object_unref(self_eas_msg_base);	
	g_debug("EAS_TYPE_MSG_BASE --");

	g_debug("EAS_TYPE_PING_MSG ++");
	self_eas_ping_msg = g_object_new (EAS_TYPE_PING_MSG, NULL);
	g_object_ref(self_eas_ping_msg);
	g_object_unref(self_eas_ping_msg);
	g_object_unref(self_eas_ping_msg);	
	g_debug("EAS_TYPE_PING_MSG --");

	g_debug("EAS_TYPE_PING_REQ ++");
	self_eas_ping_req = g_object_new (EAS_TYPE_PING_REQ, NULL);
	g_object_ref(self_eas_ping_req);
	g_object_unref(self_eas_ping_req);
	g_object_unref(self_eas_ping_req);	
	g_debug("EAS_TYPE_PING_REQ --");

	g_debug("EAS_TYPE_PROVISION_MSG ++");
	self_eas_provision_msg = g_object_new (EAS_TYPE_PROVISION_MSG, NULL);
	g_object_ref(self_eas_provision_msg);
	g_object_unref(self_eas_provision_msg);
	g_object_unref(self_eas_provision_msg);	
	g_debug("EAS_TYPE_PROVISION_MSG --");

	g_debug("EAS_TYPE_PROVISION_REQ ++");
	self_eas_provision_req = g_object_new (EAS_TYPE_PROVISION_REQ, NULL);
	g_object_ref(self_eas_provision_req);
	g_object_unref(self_eas_provision_req);
	g_object_unref(self_eas_provision_req);	
	g_debug("EAS_TYPE_PROVISION_REQ --");

	g_debug("EAS_TYPE_REQUEST_BASE ++");
	self_eas_request_base = g_object_new (EAS_TYPE_REQUEST_BASE, NULL);
	g_object_ref(self_eas_request_base);
	g_object_unref(self_eas_request_base);
	g_object_unref(self_eas_request_base);	
	g_debug("EAS_TYPE_REQUEST_BASE --");

	g_debug("EAS_TYPE_SEND_EMAIL_MSG ++");
	self_eas_send_email_msg = g_object_new (EAS_TYPE_SEND_EMAIL_MSG, NULL);
	g_object_ref(self_eas_send_email_msg);
	g_object_unref(self_eas_send_email_msg);
	g_object_unref(self_eas_send_email_msg);	
	g_debug("EAS_TYPE_SEND_EMAIL_MSG --");

	g_debug("EAS_TYPE_SEND_EMAIL_REQ ++");
	self_eas_send_email_req = g_object_new (EAS_TYPE_SEND_EMAIL_REQ, NULL);
	g_object_ref(self_eas_send_email_req);
	g_object_unref(self_eas_send_email_req);
	g_object_unref(self_eas_send_email_req);	
	g_debug("EAS_TYPE_SEND_EMAIL_REQ --");

	g_debug("EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ ++");
	self_eas_sync_folder_hierarchy_req = g_object_new (EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ, NULL);
	g_object_ref(self_eas_sync_folder_hierarchy_req);
	g_object_unref(self_eas_sync_folder_hierarchy_req);
	g_object_unref(self_eas_sync_folder_hierarchy_req);	
	g_debug("EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ --");

	g_debug("EAS_TYPE_SYNC_FOLDER_MSG ++");
	self_eas_sync_folder_msg = g_object_new (EAS_TYPE_SYNC_FOLDER_MSG, NULL);
	g_object_ref(self_eas_sync_folder_msg);
	g_object_unref(self_eas_sync_folder_msg);
	g_object_unref(self_eas_sync_folder_msg);	
	g_debug("EAS_TYPE_SYNC_FOLDER_MSG --");

	g_debug("EAS_TYPE_SYNC__MSG ++");
	self_eas_sync_msg = g_object_new (EAS_TYPE_SYNC_MSG, NULL);
	g_object_ref(self_eas_sync_msg);
	g_object_unref(self_eas_sync_msg);
	g_object_unref(self_eas_sync_msg);	
	g_debug("EAS_TYPE_SYNC_MSG --");

	g_debug("EAS_TYPE_SYNC__REQ ++");
	self_eas_sync_req = g_object_new (EAS_TYPE_SYNC_REQ, NULL);
	g_object_ref(self_eas_sync_req);
	g_object_unref(self_eas_sync_req);
	g_object_unref(self_eas_sync_req);	
	g_debug("EAS_TYPE_SYNC_REQ --");

	g_debug("EAS_TYPE_UPDATE_EMAIL_REQ ++");
	self_eas_update_email_req = g_object_new(EAS_TYPE_UPDATE_EMAIL_REQ, NULL);
	g_object_ref(self_eas_update_email_req);
	g_object_unref(self_eas_update_email_req);
	g_object_unref(self_eas_update_email_req);	
	g_debug("EAS_TYPE_UPDATE_EMAIL_REQ --");

	g_debug("EAS_TYPE_UPDATE_ITEM_REQ ++");
	self_eas_update_item_req = g_object_new(EAS_TYPE_UPDATE_ITEM_REQ, NULL);
	g_object_ref(self_eas_update_item_req);
	g_object_unref(self_eas_update_item_req);
	g_object_unref(self_eas_update_item_req);	
	g_debug("EAS_TYPE_UPDATE_ITEM_REQ --");

	g_debug("EAS_TYPE_MAIL ++");
	self_eas_mail = g_object_new(EAS_TYPE_MAIL, NULL);
	g_object_ref(self_eas_mail);
	g_object_unref(self_eas_mail);
	g_object_unref(self_eas_mail);	
	g_debug("EAS_TYPE_MAIL --");

	g_debug("EAS_TYPE_SYNC ++");
	self_eas_sync = g_object_new(EAS_TYPE_SYNC, NULL);
	g_object_ref(self_eas_sync);
	g_object_unref(self_eas_sync);
	g_object_unref(self_eas_sync);	
	g_debug("EAS_TYPE_SYNC --");

	g_debug("EAS_TYPE_TEST ++");
	self_eas_test = g_object_new(EAS_TYPE_TEST, NULL);
	g_object_ref(self_eas_test);
	g_object_unref(self_eas_test);
	g_object_unref(self_eas_test);	
	g_debug("EAS_TYPE_TEST --"); 

	g_debug("EAS_TYPE_ACCOUNT_LIST ++");
	self_eas_account_list = g_object_new(EAS_TYPE_ACCOUNT_LIST, NULL);
	g_object_ref(self_eas_account_list);
	g_object_unref(self_eas_account_list);
	g_object_unref(self_eas_account_list);	
	g_debug("EAS_TYPE_ACCOUNT_LIST --"); 

	g_debug("EAS_TYPE_ACCOUNT ++");
	self_eas_account = g_object_new(EAS_TYPE_ACCOUNT, NULL);
	g_object_ref(self_eas_account);
	g_object_unref(self_eas_account);
	g_object_unref(self_eas_account);	
	g_debug("EAS_TYPE_ACCOUNT --"); 

	g_debug("EAS_TYPE_ATTACHMENT ++");
	self_eas_attachment = g_object_new(EAS_TYPE_ATTACHMENT, NULL);
	g_object_ref(self_eas_attachment);
	g_object_unref(self_eas_attachment);
	g_object_unref(self_eas_attachment);	
	g_debug("EAS_TYPE_ATTACHMENT --");

	g_debug("EAS_TYPE_EMAIL_INFO ++");
	self_eas_email_info = g_object_new(EAS_TYPE_EMAIL_INFO, NULL);
	g_object_ref(self_eas_email_info);
	g_object_unref(self_eas_email_info);
	g_object_unref(self_eas_email_info);	
	g_debug("EAS_TYPE_EMAIL_INFO --");

	g_debug("EAS_TYPE_FOLDER ++");
	self_eas_folder = g_object_new(EAS_TYPE_FOLDER, NULL);
	g_object_ref(self_eas_folder);
	g_object_unref(self_eas_folder);
	g_object_unref(self_eas_folder);	
	g_debug("EAS_TYPE_FOLDER --");

	g_debug("EAS_TYPE_EMAIL_HANDLER ++");
	self_eas_email_handler = g_object_new(EAS_TYPE_EMAIL_HANDLER, NULL);
	g_object_ref(self_eas_email_handler);
	g_object_unref(self_eas_email_handler);
	g_object_unref(self_eas_email_handler);	
	g_debug("EAS_TYPE_EMAIL_HANDLER --");

	g_debug("EAS_TYPE_ITEM_INFO ++");
	self_eas_item_info = g_object_new(EAS_TYPE_ITEM_INFO	, NULL);
	g_object_ref(self_eas_item_info);
	g_object_unref(self_eas_item_info);
	g_object_unref(self_eas_item_info);	
	g_debug("EAS_TYPE_ITEM_INFO --");

	g_debug("EAS_TYPE_SYNC_HANDLER ++");
	self_eas_sync_handler = g_object_new(EAS_TYPE_SYNC_HANDLER	, NULL);
	g_object_ref(self_eas_sync_handler);
	g_object_unref(self_eas_sync_handler);
	g_object_unref(self_eas_sync_handler);	
	g_debug("EAS_TYPE_SYNC_HANDLER --");


	g_debug("EAS_TYPE_TEST_HANDLER ++");
	self_eas_test_handler = g_object_new(EAS_TYPE_TEST_HANDLER	, NULL);
	g_object_ref(self_eas_test_handler);
	g_object_unref(self_eas_test_handler);
	g_object_unref(self_eas_test_handler);	
	g_debug("EAS_TYPE_TEST_HANDLER --");

	g_debug("EAS_TYPE_GET_ITEM_ESTIMATE_REQ++");
	self_eas_get_item_estimate_req = g_object_new(EAS_TYPE_GET_ITEM_ESTIMATE_REQ, NULL);
	g_object_ref(self_eas_get_item_estimate_req);
	g_object_unref(self_eas_get_item_estimate_req);
	g_object_unref(self_eas_get_item_estimate_req);
	g_debug("EAS_TYPE_GET_ITEM_ESTIMATE_REQ--");

	g_debug("EAS_TYPE_GET_ITEM_ESTIMATE_MSG++");
	self_eas_get_item_estimate_msg = g_object_new(EAS_TYPE_GET_ITEM_ESTIMATE_MSG, NULL);
	g_object_ref(self_eas_get_item_estimate_msg);
	g_object_unref(self_eas_get_item_estimate_msg);
	g_object_unref(self_eas_get_item_estimate_msg);
	g_debug("EAS_TYPE_GET_ITEM_ESTIMATE_MSG--");
	
	g_debug("EAS_TYPE_2WAY_SYNC_REQ ++");
	self_eas_2way_sync_req = g_object_new (EAS_TYPE_2WAY_SYNC_REQ, NULL);
	g_object_ref(self_eas_2way_sync_req);
	g_object_unref(self_eas_2way_sync_req);
	g_object_unref(self_eas_2way_sync_req);	
	g_debug("EAS_TYPE_2WAY_SYNC_REQ --");
}
END_TEST

Suite* eas_gobjectunittest_suite (void)
{
    Suite* s = suite_create ("gobjectunittest");

    /* gobject unit test case */
    TCase *tc_gobjectunit = tcase_create ("core");
    suite_add_tcase (s, tc_gobjectunit);

	tcase_add_test (tc_gobjectunit, test_add_item_req_obj);

    return s;
}

