// https://github.com/KubaO/stackoverflown/tree/master/questions/schedule-gap-54294739
#include <QtCore>
#include <type_traits>

struct Block {
   QTime start, end;
   enum class Kind { Null, Available, Busy } kind = Kind::Null;
   Block() = default;
   Block(const Block &) = default;
   Block(const QTime &start, const QTime &end, Block::Kind kind)
       : start(start), end(end), kind(kind) {
      Q_ASSERT(start <= end);
   }
};

class Event {
  public:
   int available = 0, busy = 0;
   QTime time;
   enum class Kind { Null, BeginAvailable, EndAvailable, BeginBusy, EndBusy };
   Event() = default;
   Event(const QTime &time, Kind kind)
       : available(kind == Kind::BeginAvailable ? +1
                                                : kind == Kind::EndAvailable ? -1 : 0),
         busy(kind == Kind::BeginBusy ? +1 : kind == Kind::EndBusy ? -1 : 0),
         time(time) {}
   Block::Kind blockKind() const {
      return available ? Block::Kind::Available
                       : busy ? Block::Kind::Busy : Block::Kind::Null;
   }
};

using Blocks = QVector<Block>;
using Events = QVector<Event>;

Events eventsFromBlocks(const Blocks &);
Events sortedEvents(const Events &);
enum class MergeOp { Available, Busy, AvailableNotBusy, BusyNotAvailable };
Events mergeEvents(const Events &, MergeOp);
Blocks blocksFromEvents(const Events &);

Blocks mergeBlocks(const Blocks &a, const Blocks &b, MergeOp op) {
   auto events = eventsFromBlocks(a);
   events.append(eventsFromBlocks(b));
   events = sortedEvents(std::move(events));
   events = mergeEvents(std::move(events), op);
   return blocksFromEvents(std::move(events));
}

Blocks freeSchedule(const Blocks &a, const Blocks &b) {
   return mergeBlocks(a, b, MergeOp::AvailableNotBusy);
}

Blocks freeWorkSchedule(const Blocks &busy) {
   return freeSchedule({{{8, 0}, {17, 0}, Block::Kind::Available}}, busy);
}

Events eventsFromBlocks(const Blocks &schedule) {
   Events events;
   events.reserve(schedule.size() * 2);
   for (auto &block : schedule) {
      if (block.kind == Block::Kind::Available) {
         events.push_back({block.start, Event::Kind::BeginAvailable});
         events.push_back({block.end, Event::Kind::EndAvailable});
      } else if (block.kind == Block::Kind::Busy) {
         events.push_back({block.start, Event::Kind::BeginBusy});
         events.push_back({block.end, Event::Kind::EndBusy});
      }
   }
   return events;
}

Blocks blocksFromEvents(const Events &events) {
   Blocks blocks;
   blocks.reserve(events.size() / 2);
   bool start = true;
   for (auto &event : events) {
      if (start) {
         blocks.push_back({event.time, {}, event.blockKind()});
      } else {
         blocks.back().end = event.time;
         Q_ASSERT(blocks.back().kind == event.blockKind());
      }
      start = !start;
   }
   return blocks;
}

Events sortedEvents(const Events &events) {
   Events sorted = events;
   std::sort(sorted.begin(), sorted.end(),
             [](const Event &a, const Event &b) { return a.time < b.time; });
   return sorted;
}

template <typename Op>
std::enable_if_t<std::is_invocable_r_v<Event::Kind, Op, int, int>, Events> mergeEvents(
    const Events &events, Event::Kind recessiveKind, Op op) {
   Events merged;
   QTime prevTime;
   Event::Kind prevState = recessiveKind;
   int available = 0, busy = 0;
   for (auto ev = events.begin();; ev++) {
      if (ev != events.end()) {
         available += ev->available;
         busy += ev->busy;
      }
      Q_ASSERT(available >= 0);
      Q_ASSERT(busy >= 0);
      if (ev == events.end() || prevTime != ev->time) {
         Event::Kind state = op(available, busy);
         if (prevState != state) {
            merged.push_back({ev->time, state});
            prevState = state;
         }
      }
   }
   return events;
}

Events mergeEvents(const Events &events, MergeOp op) {
   switch (op) {
      case MergeOp::Available:
         return mergeEvents(events, Event::Kind::EndAvailable, [](int a, int) {
            return a ? Event::Kind::BeginAvailable : Event::Kind::EndAvailable;
         });
      case MergeOp::AvailableNotBusy:
         return mergeEvents(events, Event::Kind::EndAvailable, [](int a, int b) {
            return (a && !b) ? Event::Kind::BeginAvailable : Event::Kind::EndAvailable;
         });
      case MergeOp::Busy:
         return mergeEvents(events, Event::Kind::EndBusy, [](int, int b) {
            return b ? Event::Kind::BeginBusy : Event::Kind::EndBusy;
         });
      case MergeOp::BusyNotAvailable:
         return mergeEvents(events, Event::Kind::EndBusy, [](int a, int b) {
            return (b && !a) ? Event::Kind::BeginBusy : Event::Kind::EndBusy;
         });
   }
}
