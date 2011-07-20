/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "Config.hpp"
#include <MGConfItem>


MeeGo::ActiveSync::Config::Config(QString email,
				  EAccountList * accountList,
				  QObject * parent)
  : QObject(parent)
  , m_email(email.isEmpty() ? "<unspecified>" : email)
  , m_username()
  , m_password()
  , m_server_url()
  , m_username_conf(0)
  , m_password_conf(0)
  , m_server_url_conf(0)
  , m_email_account(email, accountList)
{

  QString const key =
    QString(MeeGo::ActiveSync::KEY_BASE) + "/" + m_email + "/";
  m_username_conf   = new MGConfItem(key + "username",  this);
  m_password_conf   = new MGConfItem(key + "password",  this);
  m_server_url_conf = new MGConfItem(key + "serverUri", this);

  // Connect individual MGConfItem::valueChanged() signals.
  connect(m_username_conf,
	  SIGNAL(valueChanged()),
	  this,
	  SLOT(usernameConfChanged()));

  connect(m_password_conf,
	  SIGNAL(valueChanged()),
	  this,
	  SLOT(passwordConfChanged()));

  connect(m_server_url_conf,
	  SIGNAL(valueChanged()),
	  this,
	  SLOT(serverURLConfChanged()));

  // Retrieve existing ActiveSync daemon configuration values.
  m_username   = m_username_conf->value(QString()).toString();
  m_password   = m_password_conf->value(QString()).toString();
  m_server_url = m_server_url_conf->value(QString()).toString();

  // Retrieve existing ActiveSync calendar/contacts configuration
  // values.
  // @todo ...
}

MeeGo::ActiveSync::Config::~Config()
{
}

QString
MeeGo::ActiveSync::Config::emailAddress() const
{
  return m_email;
}

QString
MeeGo::ActiveSync::Config::username() const
{
  return m_username;
}

QString
MeeGo::ActiveSync::Config::password() const
{
  return m_password;
}

QString
MeeGo::ActiveSync::Config::serverURL() const
{
  /**
   * @todo Support autodiscovery.
   */

  return m_server_url;
}

void
MeeGo::ActiveSync::Config::usernameConfChanged()
{
  QString const s = m_username_conf->value(QString()).toString();

  if (s != m_username) {
    m_username = s;
    emit usernameChanged(s);
  }
}

void
MeeGo::ActiveSync::Config::passwordConfChanged()
{
  QString const s = m_password_conf->value(QString()).toString();

  if (s != m_password) {
    m_password = s;
    emit passwordChanged(s);
  }
}

void
MeeGo::ActiveSync::Config::serverURLConfChanged()
{
  /**
   * @todo Support autodiscovery.
   */

  QString const s = m_server_url_conf->value(QString()).toString();

  if (s != m_server_url) {
    m_server_url = s;
    emit serverURLChanged(s);
  }
}

bool
MeeGo::ActiveSync::Config::writeConfig(QString username,
				       QString password,
				       QString serverURL,
				       EAccountList * accountList)
{
  // ------------------------------------------
  // Write the ActiveSync daemon configuration
  // ------------------------------------------
  if (username != m_username) {
    m_username = username;
    m_username_conf->set(username);
  }

  if (password != m_password) {
    m_password = password;
    m_password_conf->set(password);
  }

  if (serverURL != m_server_url) {
    m_server_url = serverURL;
    m_server_url_conf->set(serverURL);
  }

  // ------------------------------------------
  // Write the ActiveSync e-mail configuration
  // ------------------------------------------
  bool success = m_email_account.writeConfig(username, accountList);

  // -------------------------------------------------
  // Remove the ActiveSync SyncEvolution configuration
  // -------------------------------------------------
  // @todo ...

  return success;
}

void
MeeGo::ActiveSync::Config::removeConfig(EAccountList * accountList)
{
  // ------------------------------------------
  // Remove the ActiveSync daemon configuration
  // ------------------------------------------
  QString const key =
    QString(MeeGo::ActiveSync::KEY_BASE) + "/" + m_email;

  MGConfItem account(key);
  account.unset();

  // ------------------------------------------
  // Remove the ActiveSync e-mail configuration
  // ------------------------------------------
  m_email_account.removeConfig(accountList);

  // -------------------------------------------------
  // Remove the ActiveSync SyncEvolution configuration
  // -------------------------------------------------
  // @todo ...
}
