/****************************************************************************
**
** Copyright (C) 2017 Kuba Ober
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the "Software"),
** to deal in the Software without restriction, including without limitation
** the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the
** Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
** OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
**  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
** THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
**
****************************************************************************/

#include "stackoverflowplugin.h"
#include "stackoverflowconstants.h"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

namespace StackOverflow {
namespace Internal {

StackOverflowPlugin::StackOverflowPlugin()
{
   // Create your members
}

StackOverflowPlugin::~StackOverflowPlugin()
{
   // Unregister objects from the plugin manager's object pool
   // Delete members
}

bool StackOverflowPlugin::initialize(const QStringList &arguments, QString *errorString)
{
   // Register objects in the plugin manager's object pool
   // Load settings
   // Add actions to menus
   // Connect to other plugins' signals
   // In the initialize function, a plugin can be sure that the plugins it
   // depends on have initialized their members.

   Q_UNUSED(arguments)
   Q_UNUSED(errorString)

   QAction *action = new QAction(tr("StackOverflow Action"), this);
   Core::Command *cmd = Core::ActionManager::registerAction(action, Constants::ACTION_ID,
                                                            Core::Context(Core::Constants::C_GLOBAL));
   cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
   connect(action, &QAction::triggered, this, &StackOverflowPlugin::triggerAction);

   Core::ActionContainer *menu = Core::ActionManager::createMenu(Constants::MENU_ID);
   menu->menu()->setTitle(tr("StackOverflow"));
   menu->addAction(cmd);
   Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

   return true;
}

void StackOverflowPlugin::extensionsInitialized()
{
   // Retrieve objects from the plugin manager's object pool
   // In the extensionsInitialized function, a plugin can be sure that all
   // plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag StackOverflowPlugin::aboutToShutdown()
{
   // Save settings
   // Disconnect from signals that are not needed during shutdown
   // Hide UI (if you add UI that is not in the main window directly)
   return SynchronousShutdown;
}

void StackOverflowPlugin::triggerAction()
{
   QMessageBox::information(Core::ICore::mainWindow(),
                            tr("Action Triggered"),
                            tr("This is an action from StackOverflow."));
}

} // namespace Internal
} // namespace StackOverflow
