/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "ActiveSyncConfig.hpp"
#include <MGConfItem>

#include <QDebug>

MeeGo::ActiveSync::Config::Config(QObject* parent)
  : QObject(parent)
  , m_email_address()
  , m_username()
  , m_password()
  , m_server_url()
  , m_username_conf(0)
  , m_password_conf(0)
  , m_server_url_conf(0)
{
}

MeeGo::ActiveSync::Config::~Config()
{
}

QString
MeeGo::ActiveSync::Config::emailAddress() const
{
  return m_email_address;
}

void
MeeGo::ActiveSync::Config::setEmailAddress(QString s)
{
  if (s != m_email_address) {
    m_email_address = s;
    completeInit();
    emit emailAddressChanged(s);

    qDebug() << "************* EMAIL ADDRESS" << m_email_address;

    // @todo Where do we persistently store the e-mail address so that
    //       user isn't always prompted for it each time the
    //       ActiveSync settings UI is executed.
  }
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

void
MeeGo::ActiveSync::Config::completeInit()
{
  // If m_username_conf has been initialized all other "_conf" members
  // have also been initialized.
  if (m_username_conf != 0)
    return;

  static char const KEY_BASE[] = "/apps/activesyncd/accounts/";
  QString key(KEY_BASE + m_email_address);

  m_username_conf   = new MGConfItem(key + "/username",  this);
  m_password_conf   = new MGConfItem(key + "/password",  this);
  m_server_url_conf = new MGConfItem(key + "/serverUri", this);

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

  // Retrieve existing values.
  m_username   = m_username_conf->value(QString()).toString();
  m_password   = m_password_conf->value(QString()).toString();
  m_server_url = m_server_url_conf->value(QString()).toString();
}

bool
MeeGo::ActiveSync::Config::writeConfig(QString username,
				       QString password,
				       QString serverURL)
{
  if (m_email_address.isEmpty())
    return false;

  qDebug() << "************* USERNAME: " << username;
  qDebug() << "************* PASSWORD: " << password;
  qDebug() << "************* SERVER URL: " << serverURL;


  // If we get here we know that the MGConfItem objects have been
  // instantiated.

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

  return true;
}
