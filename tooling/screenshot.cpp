// https://github.com/KubaO/stackoverflown/tree/master/tooling

#include "backport.h"
#include "tooling.h"

#ifdef QT_WIDGETS_LIB

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#if 0  // The Qt-4 style hooks work just fine and are portable.
#include <private/qhooks_p.h>
#endif
#endif
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QElapsedTimer>
#include <QEvent>
#include <QFile>
#include <QPainter>
#include <QScreen>
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
#include <QWindow>
#endif

namespace tooling {

struct Times {
   static int constexpr collect() { return 1000; }
   static int constexpr minCollect() { return 100; }
   static int constexpr screenshotDelay() { return HostOsInfo::isMacHost() ? 250 : 500; }
};

static bool isProxied(QWidget *widget) {
   static EventLoopContext ctx;
   static QWidgetList proxied;
   Q_ASSERT(widget && widget->isWindow());

   if (ctx.needsRearm()) {
      proxied = getProxiedWidgets();
      ctx.rearm();
   }
   return proxied.contains(widget);
}

static void takeScreenshot(int n, QWidget *w) {
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

class ScreenshotTaker : public QObject {
   static int n;
   QAtomicInt eventCount = 0;
   enum Phase { Collecting, Collected, Screenshot };
   enum State { Initial, Updated, Painted, Ignored };
   Phase phase = Collecting;
   QBasicTimer timer;
   QElapsedTimer time;
   // FIXME Track widget lifetimes
   // FIXME Do not raise widgets before the event loop has started spinning
   QWidgetList topLevels;
   QVector<State> states;
   int leftToPaint = 0;
   void timerEvent(QTimerEvent *ev) override {
      if (ev->timerId() == timer.timerId()) {
         if (phase == Collecting || phase == Collected)
            scheduleScreenshots();
         else if (phase == Screenshot)
            takeScreenshots();
      }
   }
   void scheduleScreenshots() {
      qDebug() << "Deferring screenshots";
      qApp->removeEventFilter(this);
      timer.start(Times::screenshotDelay(), this);
      phase = Screenshot;
   }
   void takeScreenshots() {
      Q_ASSERT(topLevels.size() == states.size());
      for (int i = 0; i < topLevels.size(); ++i) {
         auto *const w = topLevels.at(i);
         if (!w->isWindow())
            qDebug() << "Skipping non-window" << w;
         else if (isProxied(w))
            qDebug() << "Skipping proxied widget" << w;
         else if (states.at(i) == Painted)
            takeScreenshot(++n, topLevels.at(i));
      }
      deleteLater();
   }
   bool eventFilter(QObject *o, QEvent *ev) override {
      static bool recursed;
      eventCount.fetchAndAddOrdered(1);
      if (recursed) return recursed = false;
      recursed = true;
      Q_ASSERT(topLevels.size() == states.size());
      if (o->thread() != thread() || !o->isWidgetType()) return recursed = false;
      auto *w = static_cast<QWidget *>(o);
      auto *window = w->window();
      int i = topLevels.indexOf(window);
      if (phase == Collecting && i == -1) {
         i = topLevels.size();
         topLevels.push_back(window);
         qDebug() << "Noting" << window << "for a screenshot";
         states.push_back(Initial);
         leftToPaint++;
      } else if (i >= 0) {
         auto &state = states[i];
         if (window->isVisible() && state == Initial) {
            if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0) && HostOsInfo::isMacHost()) {
               window->raise();
               qDebug() << "Raising" << window << "for a screenshot";
            }
            window->update();
            state = Updated;
         }
         if ((ev->type() == QEvent::Paint || ev->type() == QEvent::UpdateRequest) &&
             state != Painted && state != Ignored) {
            state = Painted;
            qDebug() << w << window << "painted";
            leftToPaint--;
         }
         if (leftToPaint == 0 && time.elapsed() > Times::minCollect()) {
            timer.stop();
            scheduleScreenshots();
         }
      }
      return recursed = false;
   }

  public:
   ScreenshotTaker(QWidget *parent = {}) : QObject(parent) {
      if (parent) {
         Q_ASSERT(parent->isWindow());
         phase = Collected;
         topLevels.push_back(parent);
         states.push_back(Painted);
      } else {
         time.start();
         timer.start(Times::collect(), this);
      }
      qApp->installEventFilter(this);
   }
   ~ScreenshotTaker() override {
      qDebug() << "Saw" << eventCount << "events before taking screenshots.";
   }
};  // namespace tooling

int ScreenshotTaker::n;

void takeScreenshot(QWidget *widget) {
   Q_ASSERT(widget && widget->isWindow());
   if (isProxied(widget)) {
      qDebug() << "Skipping proxied widget" << widget;
      return;
   }
   if (!widget->isVisible()) {
      qDebug() << "Skipping hidden widget" << widget;
      return;
   }
   new ScreenshotTaker(widget);
}

static void takeScreenshots() {
#ifdef NO_SCREENSHOTS
   return;
#endif
   if (qApp->property("no_screenshots").toBool()) {
      qDebug() << "Screenshot: Disabled by application property";
      return;
   }
   qDebug() << "Screenshot: Startup";
   new ScreenshotTaker();
}

static bool initialized = [] {
   detail::registerHook(detail::HasQApplicationHook, &takeScreenshots);
   return true;
}();

}  // namespace tooling

#endif  // QT_WIDGETS_LIB
