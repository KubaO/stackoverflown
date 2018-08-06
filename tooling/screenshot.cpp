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
   static PointerList<QWidget> proxied;
   Q_ASSERT(widget && !wasDeleted(widget) && widget->isWindow());

   if (ctx.needsRearm()) {
      proxied = getProxiedWidgets();
      ctx.rearm();
   }
   return proxied.contains(widget);
}

static void takeScreenshot(int n, QWidget *w) {
   auto rect = shadowlessFrameGeometry(w->frameGeometry());
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
   tooling::QSaveFile f(path.absoluteFilePath(fileName));
   if (f.open(QIODevice::WriteOnly | QIODevice::Truncate) && pix.save(&f, "PNG")) {
      qDebug() << "Took screenshot of" << w << ":" << f.fileName();
      if (!showInGraphicalShell(w, f.fileName(), n == 1))
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
   struct TopLevel {
      QPointer<QWidget> w = {};
      State state = Initial;
      TopLevel() = default;
      explicit TopLevel(QWidget *w, State state = Initial) : w(w), state(state) {}
      operator QWidget *() const { return w; }
      QWidget *operator->() const { return w; }
      bool operator==(QWidget *o) const { return o == w; }
   };
   QVector<TopLevel> topLevels;
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
      qDebug() << "Deferring screenshots. Noted" << topLevels.count() << "widgets.";
      qApp->removeEventFilter(this);
      timer.start(Times::screenshotDelay(), this);
      phase = Screenshot;
   }
   void takeScreenshots() {
      if (qApp->property("no_screenshots").toBool())
         qDebug() << "Screenshot: Disabled by application property";
      else
         for (auto &tl : qAsConst(topLevels)) {
            if (!tl.w)
               continue;
            else if (!tl->isWindow()) {
            } else if (isProxied(tl))
               qDebug() << "Skipping proxied widget" << tl;
            else if (tl.state == Painted)
               takeScreenshot(++n, tl);
         }
      deleteLater();
   }
   bool eventFilter(QObject *o, QEvent *ev) override {
      static bool recursed;
      eventCount.fetchAndAddOrdered(1);
      if (recursed) return recursed = false;
      recursed = true;
      if (o->thread() != thread() || !o->isWidgetType()) return recursed = false;
      auto *w = static_cast<QWidget *>(o);
      auto *window = w->window();
      auto i = std::find(topLevels.begin(), topLevels.end(), window);
      if (phase == Collecting && i == topLevels.end()) {
         topLevels.push_back(TopLevel(window));
         i = std::prev(topLevels.end());
         if (tooling::hasEventLoopSpunUp())
            qDebug() << "Noting" << window << "for a screenshot";
         leftToPaint++;
      } else if (i != topLevels.end()) {
         if (tooling::hasEventLoopSpunUp() && window->isVisible() &&
             i->state == Initial) {
            bool raise =
                QT_VERSION < QT_VERSION_CHECK(5, 0, 0) && HostOsInfo::isMacHost();
            qDebug() << (raise ? "Raising" : "Updating") << window << "for a screnshot";
            if (raise) window->raise();
            window->update();
            i->state = Updated;
         }
         if ((ev->type() == QEvent::Paint || ev->type() == QEvent::UpdateRequest) &&
             i->state != Painted && i->state != Ignored) {
            i->state = Painted;
            (w != window ? qDebug() << window : qDebug()) << w << "painted";
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
         topLevels.push_back(TopLevel(parent, Painted));
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

static bool takeScreenshots(const HookData &) {
#ifdef NO_SCREENSHOTS
   return;
#endif
   if (qApp->property("no_screenshots").toBool()) {
      qDebug() << "Screenshot: Disabled by application property";
      return false;
   }
   qDebug() << "Screenshot: Startup";
   new ScreenshotTaker();
   return false;
}

static bool initialized = [] {
   registerHook(HasQApplicationHook, &takeScreenshots);
   return true;
}();

}  // namespace tooling

#endif  // QT_WIDGETS_LIB
