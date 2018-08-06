// https://github.com/KubaO/stackoverflown/tree/master/tooling
/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2018 Kuba Ober
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "backport.h"
#include "tooling.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#ifdef TOOLING_HAS_WIDGETS
#include <QApplication>
#include <QMessageBox>
#endif

namespace tooling {

static QString substituteFileBrowserParameters(const QString &pre, const QString &file) {
   QString cmd;
   for (int i = 0; i < pre.size(); ++i) {
      QChar c = pre.at(i);
      if (c == QLatin1Char('%') && i < pre.size() - 1) {
         c = pre.at(++i);
         QString s;
         if (c == QLatin1Char('d')) {
            s = QLatin1Char('"') + QFileInfo(file).path() + QLatin1Char('"');
         } else if (c == QLatin1Char('f')) {
            s = QLatin1Char('"') + file + QLatin1Char('"');
         } else if (c == QLatin1Char('n')) {
            s = QLatin1Char('"') + QFileInfo(file).fileName() + QLatin1Char('"');
         } else if (c == QLatin1Char('%')) {
            s = c;
         } else {
            s = QLatin1Char('%');
            s += c;
         }
         cmd += s;
         continue;
      }
      cmd += c;
   }
   return cmd;
}

bool showInGraphicalShell(QObject *parent, const QString &pathIn, bool deselect) {
   const QFileInfo fileInfo(pathIn);
   // Mac, Windows support folder or file.
   if (HostOsInfo::isWindowsHost()) {
      if (detail::showInWindowsShell(pathIn, deselect)) return true;
      const auto explorer = QStandardPaths::findExecutable(QLatin1String("explorer.exe"));
      if (explorer.isEmpty()) {
#ifdef TOOLING_HAS_WIDGETS
         if (parent->isWidgetType())
            QMessageBox::warning(
                static_cast<QWidget *>(parent),
                QApplication::translate("Core::Internal",
                                        "Launching Windows Explorer Failed"),
                QApplication::translate(
                    "Core::Internal",
                    "Could not find explorer.exe in path to launch Windows Explorer."));
#endif
         return false;
      }
      QStringList param;
      if (!fileInfo.isDir()) param += QLatin1String("/select,");
      param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
      return QProcess::startDetached(explorer, param);
   } else if (HostOsInfo::isMacHost()) {
      QStringList scriptArgs;
      scriptArgs << QLatin1String("-e")
                 << QString::fromLatin1(
                        "tell application \"Finder\" to reveal POSIX file \"%1\"")
                        .arg(fileInfo.canonicalFilePath());
      QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
      scriptArgs.clear();
      scriptArgs << QLatin1String("-e")
                 << QLatin1String("tell application \"Finder\" to activate");
      int rc = QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
      return rc != -2 && rc != 1;
   } else {
      // we cannot select a file here, because no file browser really supports it...
      const QString folder =
          fileInfo.isDir() ? fileInfo.absoluteFilePath() : fileInfo.filePath();
      const QString app = QLatin1String("xdg-open %d");
      QProcess browserProc;
      const QString browserArgs = substituteFileBrowserParameters(app, folder);
      bool success = browserProc.startDetached(browserArgs);
      const QString error = QString::fromLocal8Bit(browserProc.readAllStandardError());
      success = success && error.isEmpty();
      return success;
   }
}

}  // namespace tooling
