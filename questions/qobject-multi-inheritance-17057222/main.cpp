// https://github.com/KubaO/stackoverflown/tree/master/questions/main.cpp
#include <QtCore>
#include <functional>
#include <type_traits>

class MySignaler : public QObject {
   Q_OBJECT
  public:
   Q_SIGNAL void mySignal();
} signaler;

#if QT_VERSION < 0x050000
class MyObjectShared;
class MyObjectHelper : public QObject {
   Q_OBJECT
   MyObjectShared *m_object;
   void (MyObjectShared::*m_slot)();

  public:
   MyObjectHelper(MyObjectShared *object, void (MyObjectShared::*slot)())
       : m_object(object), m_slot(slot) {
      QObject::connect(&signaler, SIGNAL(mySignal()), this, SLOT(slot()));
   }
   Q_SLOT void slot() { (m_object->*m_slot)(); }
};
#endif

class MyObjectShared {
   Q_DISABLE_COPY(MyObjectShared)
#if QT_VERSION < 0x050000
   MyObjectHelper helper;

  public:
   template <typename Derived>
   MyObjectShared(Derived *derived) : helper(derived, &MyObjectShared::mySlot) {}
#else
  public:
   template <typename Derived, typename = typename std::enable_if<
                                   std::is_base_of<MyObjectShared, Derived>::value>::type>
   MyObjectShared(Derived *derived) {
      QObject::connect(&signaler, &MySignaler::mySignal,
                       std::bind(&MyObjectShared::mySlot, derived));
   }
#endif

   bool baseSlotCalled = false;
   virtual void mySlot() { baseSlotCalled = true; }
};

class MyObject : public QObject, public MyObjectShared {
   Q_OBJECT
  public:
   MyObject(QObject *parent = nullptr) : QObject(parent), MyObjectShared(this) {}
   // optional, needed only in this immediately derived class if you want the slot to be a
   // real slot instrumented by Qt
#ifdef Q_MOC_RUN
   void mySlot();
#endif
};

class MyDerived : public MyObject {
  public:
   bool derivedSlotCalled = false;
   void mySlot() override { derivedSlotCalled = true; }
};

void test1() {
   MyObject base;
   MyDerived derived;
   Q_ASSERT(!base.baseSlotCalled);
   Q_ASSERT(!derived.baseSlotCalled && !derived.derivedSlotCalled);
   signaler.mySignal();
   Q_ASSERT(base.baseSlotCalled);
   Q_ASSERT(!derived.baseSlotCalled && derived.derivedSlotCalled);
}

int main(int argc, char *argv[]) {
   test1();
   QCoreApplication app(argc, argv);
   test1();
   return 0;
}

#include "main.moc"
