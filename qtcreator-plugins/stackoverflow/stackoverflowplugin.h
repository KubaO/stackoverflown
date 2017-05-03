#pragma once

#include "stackoverflow_global.h"

#include <extensionsystem/iplugin.h>

namespace StackOverflow {
namespace Internal {

class StackOverflowPlugin : public ExtensionSystem::IPlugin
{
   Q_OBJECT
   Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "StackOverflow.json")

public:
   StackOverflowPlugin();
   ~StackOverflowPlugin();

   bool initialize(const QStringList &arguments, QString *errorString);
   void extensionsInitialized();
   ShutdownFlag aboutToShutdown();

private:
   void triggerAction();
};

} // namespace Internal
} // namespace StackOverflow
