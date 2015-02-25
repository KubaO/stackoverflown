#include <functional>
#include <list>
#include <chrono>
#include <thread>
#include <iostream>

template <typename Clock = std::chrono::high_resolution_clock>
class Timers {
public:
   using clock = Clock;
   using duration = typename clock::duration;
   using time_point = typename clock::time_point;
private:
   struct Timer {
      duration const period;
      std::function<void()> const call;
      int repeats;
      time_point next;
      Timer(duration $period, int $repeats, std::function<void()> && $call) :
         period($period), call(std::move($call)), repeats($repeats) {}
   };
   std::list<Timer> m_timers;
public:
   Timers() {}
   Timers(const Timers &) = delete;
   Timers & operator=(const Timers &) = delete;
   template <typename C> void add(std::chrono::milliseconds period,
                                  int repeats, C && callable)
   {
      if (repeats) m_timers.push_back(Timer(period, repeats, callable));
   }
   enum class Missed { Skip, Emit };
   void run(Missed missed = Missed::Emit) {
      for (auto & timer : m_timers) timer.next = clock::now() + timer.period;
      while (! m_timers.empty()) {
         auto next = time_point::max();
         auto ti = std::begin(m_timers);
         while (ti != std::end(m_timers)) {
            while (ti->next <= clock::now()) {
               ti->call();
               if (--ti->repeats <= 0) {
                  ti = m_timers.erase(ti);
                  continue;
               }
               do {
                  ti->next += ti->period;
               } while (missed == Missed::Skip && ti->next <= clock::now());
            }
            next = std::min(next, ti->next);
            ++ ti;
         }
         if (! m_timers.empty()) std::this_thread::sleep_until(next);
      }
   }
};

int main(void)
{
   Timers<> timers;
   using ms = std::chrono::milliseconds;
   timers.add(ms(1000), 2, []{ std::cout << "Hello, world!" << std::endl; });
   timers.add(ms(100), 20, []{ std::cout << "*" << std::endl; });
   timers.run();
   std::cout << std::endl;
   return 0;
}

