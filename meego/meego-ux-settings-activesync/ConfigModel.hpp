/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef MEEGO_ACTIVESYNC_CONFIG_MODEL_HPP
#define MEEGO_ACTIVESYNC_CONFIG_MODEL_HPP

#include <QAbstractListModel>
#include <QList>
#include <libedataserver/e-account-list.h>


namespace MeeGo {
  namespace ActiveSync {

    class Config;

    /**
     * @class ConfigModel
     *
     * @brief The @c ConfigModel class provides QML-based
     *        configuration of potentially more than one ActiveSync
     *        account.
     *
     * @todo This class currently doesn't handle errors,
     *        autodiscovery, etc.
     */
    class ConfigModel : public QAbstractListModel
    {
      Q_OBJECT

    public:

      typedef QList<Config *> config_list_type;

      enum ConfigRoles {
	EmailAddressRole = Qt::UserRole + 1,
	UsernameRole,
	PasswordRole,
	ServerUrlRole
      };

      ConfigModel(QObject* parent = 0);
      virtual ~ConfigModel();

      virtual int rowCount (QModelIndex const & parent = QModelIndex()) const;
      virtual QVariant data (QModelIndex const & index,
			     int role = Qt::DisplayRole) const;
      virtual bool removeRows (int row,
			       int count,
			       QModelIndex const & parent = QModelIndex());

      Q_INVOKABLE void removeConfig(int row);
      Q_INVOKABLE void appendConfig(QString email,
				    QString username,
				    QString password,
				    QString serverURL);

    private:

      /// List of ActiveSync configurations.
      config_list_type m_configs;

      /// List of ActiveSync e-mail accounts.
      EAccountList * m_email_accounts;
    };

  }
}

#endif  /* MEEGO_ACTIVESYNC_CONFIG_MODEL_HPP */
