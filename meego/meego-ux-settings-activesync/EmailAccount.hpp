/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef MEEGO_ACTIVESYNC_EMAILACCOUNT_HPP
#define MEEGO_ACTIVESYNC_EMAILACCOUNT_HPP

#include <QString>
#include <libedataserver/e-account.h>
#include <libedataserver/e-account-list.h>


namespace MeeGo {
  namespace ActiveSync {

    class EmailAccount
    {
    public:

      EmailAccount(QString email, EAccountList * accountList);
      ~EmailAccount();

      bool writeConfig(QString username, EAccountList * accountList);
      void removeConfig(EAccountList * accountList);

    private:

      /// Underlying EDS EAccount object.
      EAccount * m_account;

      /// The ActiveSync user's email address.
      QString const m_email;
      
    };


  }
}


#endif  /* MEEGO_ACTIVESYNC_EMAILACCOUNT_HPP */
