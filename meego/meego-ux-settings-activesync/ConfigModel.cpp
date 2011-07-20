/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "ConfigModel.hpp"
#include "Config.hpp"
#include <MGConfItem>
#include <gconf/gconf-client.h>


MeeGo::ActiveSync::ConfigModel::ConfigModel(QObject* parent)
  : QAbstractListModel(parent)
  , m_configs()
  , m_email_accounts(0)
{
  QHash<int, QByteArray> roles;
  roles[EmailAddressRole] = "email";
  roles[UsernameRole]     = "username";
  roles[PasswordRole]     = "password";
  roles[ServerUrlRole]    = "serverUrl";
  setRoleNames(roles);

  // Initialize the Glib type system
  g_type_init ();

  // Retrieve e-mail accounts.
  GConfClient * const client = gconf_client_get_default();
  m_email_accounts = e_account_list_new(client);
  g_object_unref(client);

  // Retrieve the activesyncd configurations from GConf.
  MGConfItem accounts(MeeGo::ActiveSync::KEY_BASE);
  QList<QString> const keys = accounts.listDirs();

  typedef QList<QString>::const_iterator const_iterator;
  const_iterator const end = keys.end();
  for (const_iterator i = keys.begin(); i != end; ++i) {
    QString const & key = *i;
    if (!key.isEmpty()) {
      // Retrieve the e-mail from the key.
      // @todo There has to be a better way to do this!
      QString const email = key.right(key.count() - key.lastIndexOf("/") - 1);
      m_configs.append(new Config(email, m_email_accounts));
    }
  }

  // --------------------------------------------------
  // Retrieve ActiveSync calendar and contacts accounts
  // --------------------------------------------------
  // @todo ...
}

MeeGo::ActiveSync::ConfigModel::~ConfigModel()
{
  qDeleteAll(m_configs);
  g_object_unref(m_email_accounts);
}

int
MeeGo::ActiveSync::ConfigModel::rowCount(QModelIndex const & /* parent */) const
{
  return m_configs.size();
}

QVariant
MeeGo::ActiveSync::ConfigModel::data(QModelIndex const & index, int role) const
{
  int row = index.row();

  if (row < 0 || row >= m_configs.size())
    return QVariant();

  Config const & c = *m_configs[row];

  switch(role) {
  case EmailAddressRole:
    return c.emailAddress();

  case UsernameRole:
    return c.username();

  case PasswordRole:
    return c.password();

  case ServerUrlRole:
    return c.serverURL();

  default:
    break;
  }

  return QVariant();
}

bool
MeeGo::ActiveSync::ConfigModel::removeRows(int row,
					   int count,
					   QModelIndex const & parent)
{
  int const last = row + count - 1;  // Rows start at 0.

  if (row < 0 || count < 0 || last >= m_configs.size())
    return false;
  
  beginRemoveRows(parent, row, last);

  for (int i = row; i <= last; ++i) {
    // Remove the e-mail configuration
    // @todo Implement

    // Remove the SyncEvolution ActiveSync configuration
    // @todo Implement

    // Remove the activesyncd GConf item corresponding to this row.
    m_configs[i]->removeConfig(m_email_accounts);

    // Now remove the Config object from the list of configs.
    m_configs.removeAt(i);
  }
    
  endRemoveRows();

  return true;
}

void
MeeGo::ActiveSync::ConfigModel::removeConfig(int row)
{
  removeRow(row);
}

void
MeeGo::ActiveSync::ConfigModel::appendConfig(QString email,
					     QString username,
					     QString password,
					     QString serverURL)
{
  // ------------------------------------------
  // Write the ActiveSync daemon configuration
  // ------------------------------------------
  Config * const c = new Config(email, 0);

  // The following operations better not throw.
  c->writeConfig(username, password, serverURL, m_email_accounts);

  int const last = m_configs.count();

  beginInsertRows(QModelIndex(), last, last);
  m_configs.append(c);
  endInsertRows();
}
