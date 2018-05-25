// https://github.com/KubaO/stackoverflown/tree/master/questions/copyable-qobject-25890802
#include <QtCore>
#include <private/qobject_p.h>
#include <private/qorderedmutexlocker_p.h>
#include <type_traits>

static QBasicMutex qObjectMutexPool[131];
void (*qDestroyed)(QAbstractDeclarativeData *, QObject *) = 0;

QBasicMutex *pool() {
   void *ref = reinterpret_cast<void**>(&QAbstractDeclarativeData::destroyed);
   void *target = static_cast<void*>(&qObjectMutexPool);
}

/*
    ExtraData *extraData;    // extra data set by the user
    QThreadData *threadData; // id of the thread that owns the object

    QObjectConnectionListVector *connectionLists;

    Connection *senders;     // linked list of connections connected to this object
    Sender *currentSender;   // object currently activating the object
    mutable quint32 connectedSignals[2];

    union {
        QObject *currentChildBeingDeleted; // should only be used when QObjectData::isDeletingChildren is set
        QAbstractDeclarativeData *declarativeData; //extra data used by the declarative module
    };

    // these objects are all used to indicate that a QObject was deleted
    // plus QPointer, which keeps a separate list
    QAtomicPointer<QtSharedPointer::ExternalRefCountData> sharedRefcount;
    */

/*
 *     QObject *q_ptr;
    QObject *parent;
    QObjectList children;

    uint isWidget : 1;
    uint blockSig : 1;
    uint wasDeleted : 1;
    uint isDeletingChildren : 1;
    uint sendChildEvents : 1;
    uint receiveChildEvents : 1;
    uint isWindow : 1; //for QWindow
    uint deleteLaterCalled : 1;
    uint unused : 24;
    int postedEvents;
    QDynamicMetaObjectData *metaObject;
    QMetaObject *dynamicMetaObject() const;
    */
void swapObjects(QObject *lhs, QObject *rhs) {
   using std::swap;
   Q_ASSERT(lhs && rhs);
   auto const subst = [lhs, rhs](QObject *&ptr) {
      if (lhs == ptr)
         return (ptr = rhs), true;
      else if (rhs == ptr)
         return (ptr = lhs), true;
      return false;
   };
   auto *dl = QObjectPrivate::get(lhs);
   auto *dr = QObjectPrivate::get(rhs);
   Q_ASSERT(!dl->wasDeleted && !dl->isDeletingChildren);
   Q_ASSERT(!dr->wasDeleted && !dr->isDeletingChildren);
   QOrderedMutexLocker locker(signalSlotLock(sender),
                              signalSlotLock(receiver));


}

template <class Base>
class CObjectBase : public Base {
public:

};

using CObject = CObjectBase<QObject>;

void test() {
   CObject o;
}



namespace SO
{
constexpr static bool useQtInternals = true;
Q_STATIC_ASSERT(!useQtInternals || QT_VERSION_MAJOR == 5);

template <typename A, typename B>
struct is_same_no_ref : std::is_same<
      typename std::remove_reference<A>::type,
      typename std::remove_reference<B>::type
      > {};

template <typename T, typename A, typename B>
struct is_swap_ok : std::integral_constant<bool,
      SO::is_same_no_ref<T, A>::value &&
      SO::is_same_no_ref<T, B>::value &&
      !std::is_const<typename std::remove_reference<A>::type>::value &&
      !std::is_const<typename std::remove_reference<B>::type>::value
      > {};

struct DHelper : QObject {
   using QObject::d_ptr;
};
class ObjectPrivate : public QObjectData {
   static bool canUseImpl() {
      DHelper o;
      auto *const d = get(&o);
      if (d->sharedRefcount._q_value) // can't be preallocated
         return false;
      auto *const ref = QtSharedPointer::ExternalRefCountData::getAndRef(&o);
      ref->weakref.deref();
      if (ref != d->sharedRefcount._q_value)
         return false;
      return true;
   }
public:
   static bool canUse() {
      static bool result = useQtInternals && canUseImpl();
      return result;
   }
   static ObjectPrivate *get(const QObject *obj) {
      return static_cast<ObjectPrivate*>(static_cast<const DHelper*>(obj)->d_ptr.data());
   }
   ~ObjectPrivate() override {}
   void *extraData, *threadData, *connectionLists, *senders, *currentSender;
   mutable quint32 connectedSignals[2];
   union {
      QObject *currentChildBeingDeleted;
      void *declarativeData;
   };
   QAtomicPointer<QtSharedPointer::ExternalRefCountData> sharedRefcount;
};

inline bool checkShared(const QObject *obj) {
   bool ok = false;
   if (ObjectPrivate::canUse()) {
      auto *const ref = ObjectPrivate::get(obj)->sharedRefcount.load();
      ok = ref ? (ref->weakref.load() == 1) : true; // QObject;
   } else {
      qDebug("Allocating refcount in lieu of using internal structures.");
      auto *const ref = QtSharedPointer::ExternalRefCountData::getAndRef(obj);
      ok = ref->weakref.load() == 2; // QObject + ref
      ref->weakref.deref();
   }
   if (!ok)
      qWarning("Swapping objects that have weak or shared pointers is problematic.");
   return ok;
}

}// SO

QAtomicPointer<int>;
QObject;
QWeakPointer<QObject>;
QPointer<QObject>;

class CopyableObject : public QObject {
   Q_OBJECT
protected:
   static bool checkPairing(const QObject *lhs, const QObject *rhs) {
      if (!(!lhs->thread() || !rhs->thread() || lhs->thread() == rhs->thread())
          && (!lhs->thread() || lhs->thread() == QThread::currentThread())
          && (!rhs->thread() || rhs->thread() == QThread::currentThread()))
         return false;
      if (!lhs->metaObject()->inherits(rhs->metaObject()))
         return false;
      Q_UNUSED((SO::checkShared(lhs) && SO::checkShared(rhs)));
      return true;
   }
   static void swapImpl(CopyableObject &lhs, CopyableObject &rhs) noexcept {
      // Swap parentage: d_ptr swap doesn't do it
      const QObjectList lhsChildren = lhs.children();
      const QObjectList rhsChildren = rhs.children();
      QObject *lhsParent = lhs.parent();
      QObject *rhsParent = rhs.parent();
      for (QObject *c : lhsChildren) c->setParent(nullptr);
      for (QObject *c : rhsChildren) c->setParent(nullptr);
      lhs.setParent(nullptr);
      rhs.setParent(nullptr);
      lhs.d_ptr.swap(rhs.d_ptr);
      lhs.d_ptr->q_ptr = &lhs;
      rhs.d_ptr->q_ptr = &rhs;
      rhs.setParent(lhsParent);
      lhs.setParent(rhsParent);
      for (QObject *c : lhsChildren) c->setParent(&rhs);
      for (QObject *c : rhsChildren) c->setParent(&lhs);
   }
public:
   // c.f. https://stackoverflow.com/questions/6380862/
   //      https://stackoverflow.com/questions/18942438
   template <typename A, typename B> friend inline
   typename std::enable_if<SO::is_swap_ok<CopyableObject, A, B>::value>::type
   swap(A &&lhs, B &&rhs) {
      Q_ASSERT(checkPairing(&lhs, &rhs));
      swapImpl(lhs,  rhs);
   }
   explicit CopyableObject(QObject *parent = nullptr) : QObject(parent) {}
   CopyableObject(const CopyableObject &rhs) {
      *this = rhs;
   }
   CopyableObject(CopyableObject &&rhs) {
      swap(*this, rhs);
   }
   CopyableObject &operator=(const CopyableObject &rhs) {
      Q_ASSERT(checkPairing(this, &rhs));
      setParent(rhs.parent());
      setObjectName(rhs.objectName());
      blockSignals(rhs.signalsBlocked());
      return *this;

      swap(*this, CopyableObject(rhs));
      return *this;
   }
   CopyableObject &operator=(CopyableObject &&rhs) noexcept {
      swapImpl(*this, rhs);
      return *this;
   }
};
Q_DECLARE_METATYPE(CopyableObject)

// test harness

bool checkEquals(const CopyableObject &lhs, const CopyableObject &rhs) {
   return
         lhs.parent() == rhs.parent() &&
         lhs.objectName() == rhs.objectName() &&
         lhs.signalsBlocked() == rhs.signalsBlocked();
}

int main() {
   CopyableObject c1, c2;
   //QPointer<QObject> p1 = &c1;
   c1.setObjectName("foo");
   c2.setObjectName("bar");
   auto *c1Data = SO::ObjectPrivate::get(&c1);
   auto *c2Data = SO::ObjectPrivate::get(&c2);
   QVector<CopyableObject> v;
   v.push_back(c1);
   v.push_back(c2);
   Q_ASSERT(SO::ObjectPrivate::get(&c1) == c1Data);
   Q_ASSERT(SO::ObjectPrivate::get(&c2) == c2Data);
   Q_ASSERT(checkEquals(qAsConst(v)[0], c1));
   Q_ASSERT(checkEquals(qAsConst(v)[1], c2));
   auto v1 = QVariant::fromValue(c1);
   auto v2 = QVariant::fromValue(c2);
   Q_ASSERT(checkEquals(v1.value<CopyableObject>(), c1));
   Q_ASSERT(checkEquals(v2.value<CopyableObject>(), c2));
}
#include "main.moc"
