// https://github.com/KubaO/stackoverflown/tree/master/tooling

#include "tooling.h"

#include <QAbstractEventDispatcher>
#include <QDebug>
#include <QTime>
#ifdef QT_WIDGETS_LIB
#include <QApplication>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#endif

namespace tooling {

struct Hook {
   int type;
   void (*fun)();
   bool operator==(const Hook &o) const { return o.type == type && o.fun == fun; }
};

Q_GLOBAL_STATIC(QVector<Hook>, hooks)

namespace detail {
struct ContextTracker : public QEvent {
   EventLoopContext *ctx;
   ContextTracker(EventLoopContext *ctx) : QEvent(QEvent::None), ctx(ctx) {}
   ~ContextTracker() override {
      if (ctx) ctx->p = 0;
   }
};
}  // namespace detail

EventLoopContext::~EventLoopContext() {
   if (!needsRearm()) p->ctx = nullptr;
}

void EventLoopContext::rearm() {
   auto *dsp = QAbstractEventDispatcher::instance();
   if (dsp && needsRearm()) {
      p = new detail::ContextTracker(this);
      QCoreApplication::postEvent(dsp, p, Qt::HighEventPriority + 1);
   }
}

void showTime(const char *name) {
   auto time = QTime::currentTime().toString("HH:mm:ss.zzz");
   if (name)
      qDebug() << time << name;
   else
      qDebug() << time;
}

bool isAncestorOf(QObject *ancestor, QObject *obj) {
   while (obj && obj != ancestor) obj = obj->parent();
   return obj && obj == ancestor;
}

#ifdef QT_WIDGETS_LIB
QWidgetList getProxiedWidgets() {
   QWidgetList proxied;
   QList<const QGraphicsView *> views;
   QList<const QGraphicsScene *> scenes;
   for (const auto *window : QApplication::topLevelWidgets()) {
      if (auto *view = qobject_cast<const QGraphicsView *>(window)) views.push_back(view);
      views.append(window->findChildren<const QGraphicsView *>());
   }
   for (const auto *view : qAsConst(views))
      if (const auto *scene = view->scene())
         if (!scenes.contains(scene)) {
            scenes.append(scene);
            for (const auto *item : scene->items())
               if (auto *proxy = qgraphicsitem_cast<const QGraphicsProxyWidget *>(item))
                  if (proxy->widget()) proxied.append(proxy->widget());
         }
   return proxied;
}
#endif

static void onQApplication() {
   qDebug() << "SO Tooling: Startup";
   for (auto it = hooks()->begin(); it != hooks()->end();) {
      if (it->type == detail::HasQApplicationHook) {
         it->fun();
         it = hooks()->erase(it);
      } else
         ++it;
   }
}

static void setupCallbacks() {
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
      entered = receiver && receiver->metaObject() &&
                QLatin1String(receiver->metaObject()->className()) == "QApplication" &&
                QAbstractEventDispatcher::instance();
      if (!entered) return false;
      onQApplication();
      QInternal::unregisterCallback(QInternal::EventNotifyCallback, hook);
      entered = false;
      --recursionLevel;
      return false;  // we don't filter the event
   };
   QInternal::registerCallback(QInternal::EventNotifyCallback, hook);
}

static bool hooksInitialized = [] {
#ifndef QHOOKS_H
   setupCallbacks();
#else
   qtHookData[QHooks::Startup] = reinterpret_cast<quintptr>(&onQApplication);
#endif
   return true;
}();

namespace detail {
void registerHook(int type, void (*fun)()) {
   if (!hooks()->contains({type, fun})) hooks()->push_back({type, fun});
}
}  // namespace detail

}  // namespace tooling
