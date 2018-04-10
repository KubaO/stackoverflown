// https://github.com/KubaO/stackoverflown/tree/master/questions/appnap-49677034
#if !defined(__APPLE__)
#error This example is for macOS
#endif
#include <QtWidgets>
#include <mutex>
#include <objc/runtime.h>
#include <objc/message.h>

// see https://stackoverflow.com/a/49679984/1329652
namespace detail { struct LatencyCriticalLock {
   int count = {};
   id activity = {};
   id processInfo = {};
   id reason = {};
   std::unique_lock<std::mutex> mutex_lock() {
      init();
      return std::unique_lock<std::mutex>(mutex);
   }
private:
   std::mutex mutex;
   template <typename T> static T check(T i) {
      return (i != nil) ? i : throw std::runtime_error("LatencyCrticalLock init() failed");
   }
   void init() {
      if (processInfo != nil) return;
      auto const NSProcessInfo = check(objc_getClass("NSProcessInfo"));
      processInfo = check(objc_msgSend((id)NSProcessInfo, sel_getUid("processInfo")));
      reason = check(objc_msgSend((id)objc_getClass("NSString"), sel_getUid("alloc")));
      reason = check(objc_msgSend(reason, sel_getUid("initWithUTF8String:"), "LatencyCriticalLock"));
   }
}; }

class LatencyCriticalLock {
   static detail::LatencyCriticalLock d;
   bool locked = {};
public:
   struct NoLock {};
   LatencyCriticalLock &operator=(const LatencyCriticalLock &) = delete;
   LatencyCriticalLock(const LatencyCriticalLock &) = delete;
   LatencyCriticalLock() { lock(); }
   explicit LatencyCriticalLock(NoLock) {}
   ~LatencyCriticalLock() { unlock(); }
   void lock() {
      if (locked) return;
      auto l = d.mutex_lock();
      assert(d.count >= 0);
      if (!d.count) {
         assert(d.activity == nil);
         /* Start activity that tells App Nap to mind its own business: */
         /* NSActivityUserInitiatedAllowingIdleSystemSleep */
         /* | NSActivityLatencyCritical */
         d.activity = objc_msgSend(d.processInfo, sel_getUid("beginActivityWithOptions:reason:"),
                                   0x00FFFFFFULL | 0xFF00000000ULL, d.reason);
         assert(d.activity != nil);
      }
      d.count ++;
      locked = true;
      assert(d.count > 0 && locked);
   }
   void unlock() {
      if (!locked) return;
      auto l = d.mutex_lock();
      assert(d.count > 0);
      if (d.count == 1) {
         assert(d.activity != nil);
         objc_msgSend(d.processInfo, sel_getUid("endActivity:"), d.activity);
         d.activity = nil;
         locked = false;
      }
      d.count--;
      assert(d.count > 0 || d.count == 0 && !locked);
   }
   bool isLocked() const { return locked; }
};

detail::LatencyCriticalLock LatencyCriticalLock::d;

int main(int argc, char *argv[]) {
   struct Thread : QThread {
      bool reproduce = {};
      void run() override {
         LatencyCriticalLock lock{LatencyCriticalLock::NoLock()};
         if (!reproduce)
            lock.lock();
         const int period = 100;
         QElapsedTimer el;
         el.start();
         QTimer timer;
         timer.setTimerType(Qt::PreciseTimer);
         timer.start(period);
         connect(&timer, &QTimer::timeout, [&el]{
            auto const duration = el.restart();
            if (duration >= 1.1*period) qWarning() << duration << " ms";
         });
         QEventLoop().exec();
      }
      ~Thread() {
         quit();
         wait();
      }
   } thread;

   QApplication app{argc, argv};
   thread.reproduce = false;
   thread.start();

   QPushButton msg;
   msg.setText("Click to close");
   msg.showMinimized();
   msg.connect(&msg, &QPushButton::clicked, &msg, &QWidget::close);

   return app.exec();
}
