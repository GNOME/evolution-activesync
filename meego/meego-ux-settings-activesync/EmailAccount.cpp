/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "EmailAccount.hpp"
#include <QDir>


// ------------------------------------------------------------------

namespace {
  QString get_evolution_folder_uri(QString folder)
  {
    QString home = QDir::home().absolutePath();

    // Assumes Unix path convention!
    return
      QString("mbox:%1/.local/share/evolution/mail/local#%2").arg(home).arg(folder);
  }
}

// ------------------------------------------------------------------

MeeGo::ActiveSync::EmailAccount::EmailAccount(QString email,
					      EAccountList * accountList)
  : m_account(0)
  , m_email(email)
{
  EAccount const * const found =
    e_account_list_find(accountList,
			E_ACCOUNT_FIND_ID_ADDRESS,
			email.toUtf8());

  // @todo What's the proper way to copy the const EAccount?
  m_account = (found == 0 ? e_account_new() : const_cast<EAccount *>(found));
}

MeeGo::ActiveSync::EmailAccount::~EmailAccount()
{
  g_object_unref(m_account);
}

bool
MeeGo::ActiveSync::EmailAccount::writeConfig(QString username,
					     EAccountList * accountList)
{
  if (accountList == 0 || username.isEmpty() || m_email.isEmpty())
    return false;

  // -------------------------
  // Name
  // -------------------------
  e_account_set_string(m_account,
		       E_ACCOUNT_NAME,
		       username.toUtf8());

  // -------------------------
  // Identity
  // -------------------------
  e_account_set_string(m_account,
		       E_ACCOUNT_ID_NAME,
		       username.toUtf8());

  e_account_set_string(m_account,
		       E_ACCOUNT_ID_ADDRESS,
		       m_email.toUtf8());

  // E_ACCOUNT_ID_REPLY_TO
  // E_ACCOUNT_ID_ORGANIZATION,

  e_account_set_string(m_account,
		       E_ACCOUNT_ID_SIGNATURE,
		       "");

  // -------------------------
  // Source
  // -------------------------
  QString const source_url =
    QString("eas://%1/;sync_offline=1;account_uid=%2").arg(m_email).arg(m_email);
  e_account_set_string(m_account,
		       E_ACCOUNT_SOURCE_URL,
		       source_url.toUtf8());

  e_account_set_bool(m_account,
		     E_ACCOUNT_SOURCE_KEEP_ON_SERVER,
		     FALSE);  // gboolean, not C++ bool.

  e_account_set_bool(m_account,
		     E_ACCOUNT_SOURCE_AUTO_CHECK,
		     FALSE);

  e_account_set_int(m_account,
		    E_ACCOUNT_SOURCE_AUTO_CHECK_TIME,
		    0);

  e_account_set_bool(m_account,
		     E_ACCOUNT_SOURCE_SAVE_PASSWD,
		     FALSE);

  m_account->enabled = TRUE;

  // -------------------------
  // Transport
  // -------------------------
  QString const transport_url =
    QString("eas://%1/;account_uid=%2").arg(m_email).arg(m_email);
  e_account_set_string(m_account,
		       E_ACCOUNT_TRANSPORT_URL,
		       transport_url.toUtf8());

  e_account_set_bool(m_account,
		     E_ACCOUNT_TRANSPORT_SAVE_PASSWD,
		     FALSE);  // gboolean, not C++ bool.

  // -------------------------
  // Folder URIs
  // -------------------------
  QString const drafts_uri = get_evolution_folder_uri("Drafts");
  e_account_set_string(m_account,
		       E_ACCOUNT_DRAFTS_FOLDER_URI,
		       drafts_uri.toUtf8());

  QString const sent_uri = get_evolution_folder_uri("Sent");
  e_account_set_string(m_account,
		       E_ACCOUNT_SENT_FOLDER_URI,
		       sent_uri.toUtf8());

  // -------------------------
  // Enable or disable auto Cc/Bcc
  // -------------------------
  e_account_set_bool(m_account,
		     E_ACCOUNT_CC_ALWAYS,
		     FALSE);  // gboolean, not C++ bool.

  e_account_set_string(m_account,
		       E_ACCOUNT_CC_ADDRS,
		       "");

  e_account_set_bool(m_account,
		     E_ACCOUNT_BCC_ALWAYS,
		     FALSE);

  e_account_set_string(m_account,
		       E_ACCOUNT_BCC_ADDRS,
		       "");

  // -------------------------
  // Receipt policy
  // -------------------------
  e_account_set_int(m_account,
		    E_ACCOUNT_RECEIPT_POLICY,
		    E_ACCOUNT_RECEIPT_NEVER);

  // -------------------------
  // PGP config
  // -------------------------
  // E_ACCOUNT_PGP_KEY
  // E_ACCOUNT_PGP_HASH_ALGORITHM

  e_account_set_bool(m_account,
		     E_ACCOUNT_PGP_ENCRYPT_TO_SELF,
		     FALSE);  // gboolean, not C++ bool.

  e_account_set_bool(m_account,
		     E_ACCOUNT_PGP_ALWAYS_SIGN,
		     FALSE);

  e_account_set_bool(m_account,
		     E_ACCOUNT_PGP_NO_IMIP_SIGN,
		     FALSE);

  e_account_set_bool(m_account,
		     E_ACCOUNT_PGP_ALWAYS_TRUST,
		     FALSE);

  // -------------------------
  // SMIME config
  // -------------------------
  // E_ACCOUNT_SMIME_SIGN_KEY
  // E_ACCOUNT_SMIME_ENCRYPT_KEY
  // E_ACCOUNT_SMIME_HASH_ALGORITHM

  e_account_set_bool(m_account,
		     E_ACCOUNT_SMIME_SIGN_DEFAULT,
		     FALSE);
  e_account_set_bool(m_account,
		     E_ACCOUNT_SMIME_ENCRYPT_TO_SELF,
		     FALSE);
  e_account_set_bool(m_account,
		     E_ACCOUNT_SMIME_ENCRYPT_DEFAULT,
		     FALSE);

  // ----------------------------------------
  // Write the account to GConf.
  // ----------------------------------------
  e_account_list_add(accountList, m_account);
  e_account_list_save(accountList);

  return true;
}

void
MeeGo::ActiveSync::EmailAccount::removeConfig(EAccountList * accountList)
{
  if (accountList != 0)
    e_account_list_remove(accountList, m_account);
}
