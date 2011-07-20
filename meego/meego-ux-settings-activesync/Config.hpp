/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef MEEGO_ACTIVESYNC_CONFIG_HPP
#define MEEGO_ACTIVESYNC_CONFIG_HPP


#include "EmailAccount.hpp"
#include <QObject>


class MGConfItem;

namespace MeeGo {
  namespace ActiveSync {
    /**
     * @class Config
     *
     * @brief The @c Config class provides QML-based configuration of
     *        ActiveSync accounts.
     *
     * @todo This class currently doesn't handle errors,
     *        autodiscovery, etc.
     */
    class Config : public QObject
    {
      Q_OBJECT
      // Q_PROPERTY(QString emailAddress READ emailAddress NOTIFY emailAddressChanged)
      // Q_PROPERTY(QString username READ username NOTIFY usernameChanged)
      // Q_PROPERTY(QString password READ password NOTIFY passwordChanged)
      // Q_PROPERTY(QString serverURL READ serverURL NOTIFY serverURLChanged)

    public:

      Config(QString email,
	     EAccountList * accountList,
	     QObject * parent = 0);
      virtual ~Config();

      /// Retrieve ActiveSync e-mail address.
      QString emailAddress() const;

      /// Retrieve ActiveSync username.
      QString username() const;

      /// Retrieve ActiveSync password.
      QString password() const;

      /// Retrieve ActiveSync server URL.
      QString serverURL() const;

      /// Write the ActiveSync configuration to GConf.
      /**
       * @note This requires the e-mail address to have been set a
       *       priori.
       */
      bool writeConfig(QString username,
		       QString password,
		       QString serverURL,
		       EAccountList * accountList);

      /// Remove all ActiveSync account keys and values from GConf.
      void removeConfig(EAccountList * accountList);

    signals:

      /// Signal emitted when the ActiveSync username has changed.
      void usernameChanged(QString s);

      /// Signal emitted when the ActiveSync password has changed.
      void passwordChanged(QString s);

      /// Signal emitted when the ActiveSync server URL has changed.
      void serverURLChanged(QString s);

    private slots:

      /// Slots invoked when the corresponding GConf value has
      /// changed.
      //@{
      void usernameConfChanged();
      void passwordConfChanged();
      void serverURLConfChanged();
      //@}

    private:

      /// E-mail address GConf bridge.
      QString const m_email;

      /// Username GConf bridge.
      QString m_username;

      /// Password GConf bridge.
      /**
       * @todo Rely on SSO to query and set password instead.
       */
      QString m_password;

      /// Server URL GConf bridge.
      /**
       * @todo Support autodiscovery.
       */
      QString m_server_url;

      /// GConf counterparts to the above attributes.
      //@{
      MGConfItem* m_username_conf;
      MGConfItem* m_password_conf;
      MGConfItem* m_server_url_conf;
      //@}

      /// E-mail account configuration.
      EmailAccount m_email_account;

      /// SyncEvolution account configuration.
      /// @todo ...

    };

    // Base of all ActiveSync GConf keys.
    char const KEY_BASE[] = "/apps/activesyncd/accounts";
  }
}

#endif  /* MEEGO_ACTIVESYNC_CONFIG_HPP */
