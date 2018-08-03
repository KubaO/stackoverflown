// https://github.com/KubaO/stackoverflown/tree/master/tooling
/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2016 Intel Corporation.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "backport.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif
#if QT_HAS_INCLUDE(<paths.h>)
#include <paths.h>
#endif

namespace tooling {

static QStringList executableExtensions() {
   // If %PATHEXT% does not contain .exe, it is either empty, malformed, or distorted in
   // ways that we cannot support, anyway.
   const QStringList pathExt =
       QString::fromLocal8Bit(qgetenv("PATHEXT")).toLower().split(QLatin1Char(';'));
   return pathExt.contains(QLatin1String(".exe"), Qt::CaseInsensitive)
              ? pathExt
              : QStringList() << QLatin1String(".exe") << QLatin1String(".com")
                              << QLatin1String(".bat") << QLatin1String(".cmd");
}

static QString checkExecutable(const QString &path) {
   const QFileInfo info(path);
   if (info.isBundle()) return info.bundleName();
   if (info.isFile() && info.isExecutable()) return QDir::cleanPath(path);
   return QString();
}

static inline QString searchExecutable(const QStringList &searchPaths,
                                       const QString &executableName) {
   const QDir currentDir = QDir::current();
   for (const QString &searchPath : searchPaths) {
      const QString candidate =
          currentDir.absoluteFilePath(searchPath + QLatin1Char('/') + executableName);
      const QString absPath = checkExecutable(candidate);
      if (!absPath.isEmpty()) return absPath;
   }
   return QString();
}

static inline QString searchExecutableAppendSuffix(const QStringList &searchPaths,
                                                   const QString &executableName,
                                                   const QStringList &suffixes) {
   const QDir currentDir = QDir::current();
   for (const QString &searchPath : searchPaths) {
      const QString candidateRoot =
          currentDir.absoluteFilePath(searchPath + QLatin1Char('/') + executableName);
      for (const QString &suffix : suffixes) {
         const QString absPath = checkExecutable(candidateRoot + suffix);
         if (!absPath.isEmpty()) return absPath;
      }
   }
   return QString();
}

QString QStandardPathsImpl::findExecutable(const QString &executableName,
                                           const QStringList &paths) {
   if (QFileInfo(executableName).isAbsolute()) return checkExecutable(executableName);
   QStringList searchPaths = paths;
   if (paths.isEmpty()) {
      QByteArray pEnv = qgetenv("PATH");
      if (Q_UNLIKELY(pEnv.isNull())) {
         // Get a default path. POSIX.1 does not actually require this, but
         // most Unix libc fall back to confstr(_CS_PATH) if the PATH
         // environment variable isn't set. Let's try to do the same.
#if defined(_PATH_DEFPATH)
         // BSD API.
         pEnv = _PATH_DEFPATH;
#elif defined(_CS_PATH)
         // POSIX API.
         size_t n = confstr(_CS_PATH, nullptr, 0);
         if (n) {
            pEnv.resize(n);
            // size()+1 is ok because QByteArray always has an extra NUL-terminator
            confstr(_CS_PATH, pEnv.data(), pEnv.size() + 1);
         }
#else
         // Windows SDK's execvpe() does not have a fallback, so we won't
         // apply one either.
#endif
      }
      // Remove trailing slashes, which occur on Windows.
      const QStringList rawPaths = QString::fromLocal8Bit(pEnv.constData())
                                       .split(listSeparator(), QString::SkipEmptyParts);
      searchPaths.reserve(rawPaths.size());
      for (const QString &rawPath : rawPaths) {
         QString cleanPath = QDir::cleanPath(rawPath);
         if (cleanPath.size() > 1 && cleanPath.endsWith(QLatin1Char('/')))
            cleanPath.truncate(cleanPath.size() - 1);
         searchPaths.push_back(cleanPath);
      }
   }
   if (HostOsInfo::isWindowsHost()) {
      // On Windows, if the name does not have a suffix or a suffix not
      // in PATHEXT ("xx.foo"), append suffixes from PATHEXT.
      static const QStringList executable_extensions = executableExtensions();
      if (executableName.contains(QLatin1Char('.'))) {
         const QString suffix = QFileInfo(executableName).suffix();
         if (suffix.isEmpty() || !executable_extensions.contains(
                                     QLatin1Char('.') + suffix, Qt::CaseInsensitive))
            return searchExecutableAppendSuffix(searchPaths, executableName,
                                                executable_extensions);
      } else {
         return searchExecutableAppendSuffix(searchPaths, executableName,
                                             executable_extensions);
      }
   }
   return searchExecutable(searchPaths, executableName);
}

}  // namespace tooling
