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
#include <QtDebug>


MeeGo::ActiveSync::ConfigModel::ConfigModel(QObject* parent)
  : QAbstractListModel(parent)
  , m_configs()
{
  QHash<int, QByteArray> roles;
  roles[EmailAddressRole] = "email";
  roles[UsernameRole]     = "username";
  roles[PasswordRole]     = "password";
  roles[ServerUrlRole]    = "serverUrl";
  setRoleNames(roles);

  MGConfItem accounts(MeeGo::ActiveSync::KEY_BASE);
  QList<QString> const keys = accounts.listDirs();

  qDebug() << "NUM KEYS = " << keys.size();

  typedef QList<QString>::const_iterator const_iterator;
  const_iterator const end = keys.end();
  for (const_iterator i = keys.begin(); i != end; ++i) {
    QString const & key = *i;
    if (!key.isEmpty()) {
      // Retrieve the e-mail from the key.
      // @todo There has to be a better way to do this!
      QString const email = key.right(key.count() - key.lastIndexOf("/") - 1);
      m_configs.append(new Config(email));
    }
  }
}

MeeGo::ActiveSync::ConfigModel::~ConfigModel()
{
  qDeleteAll(m_configs);
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

void
MeeGo::ActiveSync::ConfigModel::appendConfig(QString email,
					     QString username,
					     QString password,
					     QString serverURL)
{
  Config * const c = new Config(email);

  // These following operations better not throw.
  c->writeConfig(username, password, serverURL);

  int const last = m_configs.count();

  beginInsertRows(QModelIndex(), last, last);
  m_configs.append(c);
  endInsertRows();
}
