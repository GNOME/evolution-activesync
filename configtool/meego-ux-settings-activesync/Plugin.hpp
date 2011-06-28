/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef MEEGO_ACTIVESYNC_UI_PLUGIN_HPP
#define MEEGO_ACTIVESYNC_UI_PLUGIN_HPP

#include <QDeclarativeExtensionPlugin>

namespace MeeGo
{
  namespace ActiveSync
  {

    /**
     * @class Plugin
     *
     * @brief The class that registers all C++ types to be exposed
     *        to QML with the metatype system.
     */
    class Plugin : public QDeclarativeExtensionPlugin
    {
      Q_OBJECT

    public:

      /// Register all activesync related types to be exposed to QML.
      virtual void registerTypes(char const * uri);

    };

  }
}

#endif /* MEEGO_ACTIVESYNC_UI_PLUGIN_HPP */
