// main.cpp
#include <QtWidgets>

template <class Base, class Derived> class MyGenericView : public Base {
   inline Derived* dthis() { return static_cast<Derived*>(this); }
public:
   bool slot1Invoked, slot2Invoked, baseSlot3Invoked;
   MyGenericView(QWidget * parent = 0) : Base(parent),
      slot1Invoked(false), slot2Invoked(false), baseSlot3Invoked(false)
   {
      QObject::connect(dthis(), &Derived::mySignal, dthis(), &Derived::mySlot2); // Qt 5 style
      QObject::connect(dthis(), &Derived::mySignal, dthis(), &Derived::mySlot3);
   }
   void doConnections() {
      Q_ASSERT(qobject_cast<Derived*>(this)); // we must be of correct type at this point
      QObject::connect(this, SIGNAL(mySignal()), SLOT(mySlot1())); // Qt 4 style
   }
   void mySlot1() { slot1Invoked = true; }
   void mySlot2() { slot2Invoked = true; }
   virtual void mySlot3() { baseSlot3Invoked = true; }
   void emitMySignal() {
      emit dthis()->mySignal();
   }
};

class MyTreeWidget : public MyGenericView<QTreeWidget, MyTreeWidget> {
   Q_OBJECT
public:
   bool slot3Invoked;
   MyTreeWidget(QWidget * parent = 0) : MyGenericView(parent), slot3Invoked(false) { doConnections(); }
   Q_SIGNAL void mySignal();
#ifdef Q_MOC_RUN // for slots not overridden here
   Q_SLOT void mySlot1();
   Q_SLOT void mySlot2();
#endif
   // visible to the C++ compiler since we override it
   Q_SLOT void mySlot3() Q_DECL_OVERRIDE { slot3Invoked = true; }
};

class LaterTreeWidget : public MyTreeWidget {
   Q_OBJECT
public:
   void mySlot3() Q_DECL_OVERRIDE { } // no Q_SLOT macro - it's already a slot!
};

class MyTableWidget : public MyGenericView<QTreeWidget, MyTableWidget> {
   Q_OBJECT
public:
   MyTableWidget(QWidget * parent = 0) : MyGenericView(parent) { doConnections(); }
   Q_SIGNAL void mySignal();
#ifdef Q_MOC_RUN
   Q_SLOT void mySlot1();
   Q_SLOT void mySlot2();
   Q_SLOT void mySlot3(); // for MOC only since we don't override it
#endif
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   MyTreeWidget tree;
   MyTableWidget table;
   Q_ASSERT(!tree.slot1Invoked && !tree.slot2Invoked && !tree.slot3Invoked);
   emit tree.mySignal();
   Q_ASSERT(tree.slot1Invoked && tree.slot2Invoked && tree.slot3Invoked);
   Q_ASSERT(!table.slot1Invoked && !table.slot2Invoked && !table.baseSlot3Invoked);
   emit table.mySignal();
   Q_ASSERT(table.slot1Invoked && table.slot2Invoked && table.baseSlot3Invoked);
   return 0;
}

#include "main.moc"

