/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "Plugin.hpp"
#include "ConfigModel.hpp"

#include <qdeclarative.h>


void
MeeGo::ActiveSync::Plugin::registerTypes(char const * uri)
{
  static int const MAJOR_VERSION = 0;
  static int const MINOR_VERSION = 1;

  // Register the class that exposes the ActiveSync configuration
  // operations to the QML with the metatype system.
  qmlRegisterType<ConfigModel>(uri,
			       MAJOR_VERSION,
			       MINOR_VERSION,
			       "ActiveSyncConfigModel");
}


Q_EXPORT_PLUGIN2(activesyncsettingsplugin, MeeGo::ActiveSync::Plugin);
