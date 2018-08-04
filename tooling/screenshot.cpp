// https://github.com/KubaO/stackoverflown/tree/master/tooling

#include "backport.h"
#include "tooling.h"

#ifdef QT_WIDGETS_LIB

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QFile>
#include <QPainter>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#if 0  // The Qt-4 style hooks work just fine and are portable.
#include <private/qhooks_p.h>
#endif
#include <QScreen>
#include <QWindow>
#endif

namespace tooling {

static int constexpr screenshotDelay() { return HostOsInfo::isMacHost() ? 250 : 500; }

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

      if ((1)) {
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

void takeScreenshot(QWidget *widget) {
   static int n = 1;
   static EventLoopContext ctx;
   static QWidgetList proxied;

   Q_ASSERT(widget && widget->isWindow());

   if (ctx.needsRearm()) {
      proxied = getProxiedWidgets();
      ctx.rearm();
   }
   if (proxied.contains(widget)) {
      qDebug() << "Skipping proxied widget" << widget;
      return;
   }
   if (!widget->isVisible()) {
      qDebug() << "Skipping hidden widget" << widget;
      return;
   }
   if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0) && HostOsInfo::isMacHost()) {
      widget->raise();
      qDebug() << "Raising" << widget << "for a screenshot";
   }
   new ScreenshotTaker(n++, widget);
}

static void takeScreenshots() {
   if (qApp->property("no_screenshots").toBool()) {
      qDebug() << "Screenshot: Disabled by application property";
      return;
   }
   int count = 0;
   for (auto *widget : QApplication::topLevelWidgets()) {
      takeScreenshot(widget);
      count++;
   }
   qDebug() << "Screenshot: Enumerated" << count << "widgets";
}

static void startupHook() {
#ifdef NO_SCREENSHOTS
   return;
#endif
   qDebug() << "Screenshot: Startup";
   tooling::QTimer::singleShot(100, qApp, takeScreenshots);
}

static bool initialized = [] {
   detail::registerHook(detail::HasQApplicationHook, &startupHook);
   return true;
}();

}  // namespace tooling

#endif  // QT_WIDGETS_LIB
