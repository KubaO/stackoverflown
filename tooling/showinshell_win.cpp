// https://github.com/KubaO/stackoverflown/tree/master/tooling
#include "tooling.h"

#ifdef Q_OS_WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <QAxObject>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QTextBrowser>
#include <QUuid>
#include <ShObjIdl.h>
#include <ShlDisp.h>
#include <Windows.h>

namespace tooling {

static void showDoc(QAxObject *obj) {
   if (!obj) return;
   auto *br = new QTextBrowser;
   br->setText(obj->generateDocumentation());
   br->show();
}

static QStringList getVerbNames(QAxBase *obj) {
   QStringList names;
   auto *verbs = obj->querySubObject("Verbs()");
   verbs->disableMetaObject();
   auto count = verbs->dynamicCall("Count()").toInt();
   for (int i = 0; i < count; ++i) {
      auto verb = verbs->querySubObject("Item(QVariant)", i);
      verb->disableMetaObject();
      names.push_back(verb->dynamicCall("Name()").toString());
   }
   return names;
}

namespace detail {
bool showInWindowsShell(const QString &filePath, bool deselect) {
   QFileInfo appFI(filePath);
   auto matchPath = appFI.dir().path().toLower();
   auto matchName = appFI.fileName().toLower();

   QAxObject shellApp("Shell.Application");

   QAxObject *windows = shellApp.querySubObject("Windows()");
   windows->disableMetaObject();
   auto count = windows->dynamicCall("Count()").toInt();
   qDebug() << count;
   for (int i = 0; i < count; ++i) {
      QAxObject *win = windows->querySubObject("Item(QVariant)", {i});
      win->disableMetaObject();

      auto program = win->dynamicCall("FullName()").toString();
      QFileInfo programFI(program);
      if (programFI.baseName().toLower() != "explorer") continue;
      auto url = win->dynamicCall("LocationURL()").toUrl();
      if (!url.isLocalFile()) continue;
      auto path = url.path().mid(1).toLower();
      if (path != matchPath) continue;

      QAxObject *doc = win->querySubObject("Document()");

      QAxObject *folder = doc->querySubObject("Folder()");
      folder->disableMetaObject();
      QAxObject *folderItems = folder->querySubObject("Items()");
      folderItems->disableMetaObject();

      QAxObject *ourEntry = {};
      int count = folderItems->dynamicCall("Count()").toInt();
      for (int j = 0; j < count; j++) {
         QAxObject *entry = folderItems->querySubObject("Item(QVariant)", j);
         entry->disableMetaObject();
         auto name = entry->dynamicCall("Name()").toString().toLower();
         if (name == matchName) ourEntry = entry;
      }
      if (ourEntry) {
         if (false)
            ourEntry->dynamicCall("InvokeVerb(QVariant)", QVariant());  // open etc.
         auto rc = doc->dynamicCall("SelectItem(QVariant, int)", ourEntry->asVariant(),
                                    SVSI_SELECT | (deselect ? SVSI_DESELECTOTHERS : 0));
         auto hwnd = win->dynamicCall("HWND()").toLongLong();
         BringWindowToTop(HWND(hwnd));
         return true;
      }
   }
   return false;
}
}  // namespace detail
}  // namespace tooling

#else

namespace tooling {
namespace detail {
bool showInWindowsShell(const QString &, bool) { return false; }
}  // namespace detail
}  // namespace tooling

#endif
