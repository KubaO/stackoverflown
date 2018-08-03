// https://github.com/KubaO/stackoverflown/tree/master/tooling
#pragma once

#include <QString>

#ifndef QStringLiteral
#define QStringLiteral QString
#endif

class QObject;
class QString;
class QWidget;

namespace tooling {

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
bool showInGraphicalShell(QObject *parent, const QString &pathIn);
void takeScreenshot(QWidget *widget);

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

namespace detail {

enum { HasQApplicationHook };

void registerHook(int type, void (*)());

}  // namespace detail

}  // namespace tooling
