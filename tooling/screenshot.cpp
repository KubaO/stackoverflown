// https://github.com/KubaO/stackoverflown/tree/master/tooling

#include "backport.h"
#include "tooling.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QFile>
#include <QPainter>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <private/qhooks_p.h>
#include <QScreen>
#include <QWindow>
#endif

namespace tooling {

static int constexpr screenshotDelay() {
   return HostOsInfo::isMacHost() ? 250 : 500;
}

class ScreenshotTaker : public QObject {
   int const n;
   bool updated = false;
   QAtomicInt eventCount = 0;
   QBasicTimer timer;
   void timerEvent(QTimerEvent *ev) override {
      if (ev->timerId() == timer.timerId()) {
         timer.stop();
         take(n, static_cast<QWidget *>(parent()));
         deleteLater();
      }
   }
   bool eventFilter(QObject *o, QEvent *ev) override {
      eventCount.fetchAndAddOrdered(1);
      if (o->isWidgetType() && isAncestorOf(parent(), o)) {
         auto *w = static_cast<QWidget *>(parent());
         if (ev->type() == QEvent::Paint && w->isActiveWindow() && !timer.isActive()) {
            qDebug() << "Deferring screenshot for" << w;
            timer.start(screenshotDelay(), this);
         } else if (!updated && ev->type() != QEvent::Paint && w->isActiveWindow()) {
            w->update();
            updated = true;
         }
      }
      return false;
   }

  public:
   ScreenshotTaker(int n, QObject *parent) : QObject(parent), n(n) {
      Q_ASSERT(parent->isWidgetType());
      qApp->installEventFilter(this);
   }
   ~ScreenshotTaker() override {
      qDebug() << "Saw" << eventCount << "events before taking a screenshot.";
   }
   static void take(int n, QWidget *w) {
      auto rect = w->frameGeometry();
      if (HostOsInfo::isMacHost()) rect.adjust(-1, -1, 1, 1);
      QPixmap pix;
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
      auto *const win = w->windowHandle();
      auto *const screen = win->screen();
      pix = screen->grabWindow(0);
#endif
      if (QT_VERSION < QT_VERSION_CHECK(5, 3, 0))
         pix = QPixmap::grabWindow(QApplication::desktop()->winId());

      if (1) {
         pix = pix.copy(rect);
      } else {
         QPainter p(&pix);
         p.drawRect(w->frameGeometry());
         p.end();
      }
      if (pix.isNull()) {
         qWarning() << "Discarding null screenshot of" << w;
         return;
      }
      auto const name = w->objectName();
      auto const fileName =
          QStringLiteral("screenshot_%1_%2%3.png")
              .arg(n)
              .arg(QLatin1String(w->metaObject()->className()))
              .arg(name.isNull() ? QString() : QStringLiteral("_%1").arg(name));
      QDir path = QCoreApplication::applicationDirPath();
      if (HostOsInfo::isMacHost() && path.path().endsWith(".app/Contents/MacOS")) {
         // store the widget in the app-containing folder, not in the bundle itself
         path.cdUp();
         path.cdUp();
         path.cdUp();
      }
      QFile f(path.absoluteFilePath(fileName));
      if (f.open(QIODevice::WriteOnly | QIODevice::Truncate) && pix.save(&f, "PNG")) {
         qDebug() << "Took screenshot" << f.fileName();
         if (!showInGraphicalShell(w, f.fileName()))
            qWarning()
                << "Can't invoke the graphical file shell to show screenshot location.";
      } else {
         qWarning() << "Can't save screenshot" << f.fileName()
                    << "Error:" << f.errorString();
      }
   }
};

static void takeScreenshots() {
   int n = 1;
   for (auto *widget : QApplication::topLevelWidgets()) {
      Q_ASSERT(widget);
      if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0) && HostOsInfo::isMacHost()) {
         widget->raise();
         qDebug() << "Raising" << widget << "for a screenshot";
      }
      new ScreenshotTaker(n, widget);
   }
}

static void startupHook() {
   qDebug() << "SO Tooling: Startup";
   tooling::QTimer::singleShot(0, qApp, takeScreenshots);
}

static void registerCallback() {
   static qInternalCallback const hook = [](void **data) {
      static int recursionLevel;
      static bool entered;
      auto *const receiver = reinterpret_cast<const QObject *>(data[0]);
      if (recursionLevel > 10) {
         qWarning() << "Tooling startup callback: recursed too deep, level is "
                    << recursionLevel;
         entered = true;
      }
      if (entered) return false;
      recursionLevel++;
      entered = qobject_cast<const QApplication *>(receiver) &&
                QAbstractEventDispatcher::instance();
      if (!entered) return false;
      startupHook();
      QInternal::unregisterCallback(QInternal::EventNotifyCallback, hook);
      entered = false;
      --recursionLevel;
      return false;  // we don't filter the event
   };
   QInternal::registerCallback(QInternal::EventNotifyCallback, hook);
}

static bool hooksInitialized = [] {
   registerCallback();
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
   if (0) qtHookData[QHooks::Startup] = reinterpret_cast<quintptr>(&startupHook);
#endif
   return true;
}();

}  // namespace tooling
