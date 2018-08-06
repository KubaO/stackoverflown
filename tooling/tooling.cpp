// https://github.com/KubaO/stackoverflown/tree/master/tooling

#include "tooling.h"
#include "backport.h"

#include <QAbstractEventDispatcher>
#include <QDebug>
#include <QScopedValueRollback>
#include <QTime>
#include <limits>
#ifdef QT_WIDGETS_LIB
#include <QApplication>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#endif

namespace tooling {

namespace detail {
struct Hook {
   using Fun = std::function<bool(const HookData &)>;
   using FunPtr = bool (*)(const HookData &);
   HookTypes types;
   Fun fun;
   const FunPtr *funTarget() const { return fun.target<FunPtr>(); }
   bool has(const Fun &o) const {
      auto *oTarget = o.target<FunPtr>();
      return funTarget() && oTarget && *funTarget() == *oTarget;
   }
};

struct ContextTracker : public QEvent {
   EventLoopContext *ctx;
   ContextTracker(EventLoopContext *ctx) : QEvent(QEvent::None), ctx(ctx) {}
   ~ContextTracker() override {
      if (ctx) ctx->p = nullptr;
   }
};

struct ObjectHelper : QObject {
   static const QObjectData *d(const QObject *o) {
      return static_cast<const ObjectHelper *>(o)->d_ptr.data();
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

bool wasDeleted(const QObject *object) {
   return detail::ObjectHelper::d(object)->wasDeleted;
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

struct CallbackProcessor {
   QVector<detail::Hook> hooks;
   HookTypes fired = nullptr;
   QEvent *loopEvent = nullptr;

   bool callHooks(HookData &d) {
      bool hookFilter = false;
      HookTypes now = d.types;
      if (!(fired & HasQApplicationHook) &&
          QLatin1String(d.receiver->metaObject()->className()) == "QApplication" &&
          QAbstractEventDispatcher::instance())
         now |= HasQApplicationHook;
      if (!(fired & EventLoopSpinupHook) && d.event == loopEvent)
         now |= EventLoopSpinupHook;
      if (!now) return hookFilter;
      fired |= now;
      d.types = now;
      if (now & HasQApplicationHook) {
         qDebug() << "SO Tooling: QApplication Startup";
         loopEvent = new QEvent(QEvent::None);
         QCoreApplication::postEvent(qApp, loopEvent, std::numeric_limits<int>::max());
      }
      if (now & EventLoopSpinupHook) {
         qDebug() << "SO Tooling: Event Loop Spins";
         loopEvent = nullptr;
         hookFilter = true;
         *d.filter = true;  // we don't want the application to see the event
      }
      for (auto it = hooks.begin(); it != hooks.end();) {
         if (it->types & d.types) {
            it->types &= (~d.types) | EventHook;
            if (!it->types) {
               auto fun = std::move(it->fun);
               it = hooks.erase(it);
               fun(d);
            } else
               it->fun(d);
         } else
            ++it;
      }
      if (hooks.isEmpty() && (fired & AllSingleShotHooks) == AllSingleShotHooks)
         QInternal::unregisterCallback(QInternal::EventNotifyCallback, eventHookStatic);
      return hookFilter;
   }

   bool eventHook(void **data) {
      static bool entered;
      Q_ASSERT(!entered);
      QScopedValueRollback<bool> rb(entered, true);
      HookData d;
      memcpy(&d, data, 3 * sizeof(void *));
      d.types = EventHook;
      return callHooks(d);
   }

   void registerHook(HookTypes types, const detail::Hook::Fun &hook) {
      auto h = hooks.begin();
      for (; h != hooks.end(); h++)
         if (h->has(hook)) break;
      if (h == hooks.end()) return hooks.push_back({types, hook});
      h->types |= types;
   }

   static bool eventHookStatic(void **data);

   void init() const {
      QInternal::registerCallback(QInternal::EventNotifyCallback, eventHookStatic);
   }
};

Q_GLOBAL_STATIC(CallbackProcessor, processor)

bool CallbackProcessor::eventHookStatic(void **data) {
   return processor()->eventHook(data);
}

void registerHook(HookTypes types, const detail::Hook::Fun &hook) {
   processor()->registerHook(types, hook);
}

static bool hooksInitialized = [] {
#ifndef QHOOKS_H
   processor()->init();
#else
   qtHookData[QHooks::Startup] = reinterpret_cast<quintptr>(&onQApplication);
#endif
   return true;
}();

}  // namespace tooling
