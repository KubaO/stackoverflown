// https://github.com/KubaO/stackoverflown/tree/master/tooling
#pragma once

#include <QPointer>
#include <QString>
#ifdef QT_WIDGETS_LIB
#include <QWidget>
#endif
#include <algorithm>

#ifndef QStringLiteral
#define QStringLiteral QString
#endif

class QObject;
class QString;
class QWidget;

namespace tooling {

template <typename T>
class PointerList : public QList<QPointer<T>> {
   using base = QList<QPointer<T>>;

  public:
   PointerList() = default;
   PointerList(PointerList &&) = default;
   PointerList(const PointerList &) = default;
   PointerList(const QList<T*> &o) {
      reserve(o.size());
      std::copy(o.begin(), o.end(), std::back_inserter(*this));
   }
   PointerList &operator=(const PointerList &o) {
      this->~PointerList();
      return *new (this) PointerList(o);
   }
};

namespace detail {
struct ContextTracker;
}

struct HostOsInfo {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) && defined(QT_WIDGETS_LIB) || \
    QT_VERSION < QT_VERSION_CHECK(5, 0, 0) && defined(QT_GUI_LIB)
#define TOOLING_HAS_WIDGETS
   static constexpr bool hasWidgets() { return true; }
#else
   static constexpr bool hasWidgets() { return false; }
#endif
#ifdef Q_OS_MAC
   static constexpr bool isMacHost() { return true; }
#else
   static constexpr bool isMacHost() { return false; }
#endif
#ifdef Q_OS_WIN
   static constexpr bool isWindowsHost() { return true; }
#else
   static constexpr bool isWindowsHost() { return false; }
#endif
};

void showTime(const char *name = {});
bool isAncestorOf(QObject *ancestor, QObject *obj);
bool wasDeleted(const QObject *);
bool showInGraphicalShell(QObject *parent, const QString &pathIn);

class EventLoopContext {
   Q_DISABLE_COPY(EventLoopContext)
   friend struct detail::ContextTracker;
   detail::ContextTracker *p = nullptr;

  public:
   EventLoopContext() = default;
   ~EventLoopContext();
   bool needsRearm() const { return !p; }
   void rearm();
};

#ifdef QT_WIDGETS_LIB
void takeScreenshot(QWidget *widget);
QWidgetList getProxiedWidgets();
#endif

namespace detail {

enum { HasQApplicationHook };

void registerHook(int type, void (*)());

}  // namespace detail

}  // namespace tooling
