// https://github.com/KubaO/stackoverflown/tree/master/questions/qobject-sep-concerns-55046944
#include <QtWidgets>

class ParentTracker : public QObject {
   QMetaObject::Connection connection;
   QObject *subjectParent = nullptr;
   inline QObject *subject() const { return parent(); }
   bool eventFilter(QObject *receiver, QEvent *event) override {
      qDebug() << receiver << event->type();
      if (receiver == subject()) {
         // Track parent changes on the child
         if (subject()->parent() != subjectParent) {
            detachFromParent();
            attachToParent();
         }
      } else if (event->type() == QEvent::ChildRemoved) {
         // Track child changes on the parent
         Q_ASSERT(receiver == subjectParent);
         auto *ev = static_cast<QChildEvent *>(event);
         if (ev->child() == subject()) {
            detachFromParent();
         }
      }
      return false;
   }
   void lostParent() {
      subject()->setParent(nullptr);
      detachFromParent();
   }
   void detachFromParent() {
      if (subjectParent) {
         disconnect(connection);
         connection = {};  // free the connection handle immediately
         subjectParent->removeEventFilter(this);
         subjectParent = nullptr;
      }
   }
   void attachToParent() {
      Q_ASSERT(!subjectParent);
      subjectParent = subject()->parent();
      bool snoopChild = !subjectParent;
      {
         auto *widget = qobject_cast<QWidget *>(subject());
         snoopChild = snoopChild ||
                      (widget && widget->testAttribute(Qt::WA_NoChildEventsForParent));
      }

      if (subjectParent) {
         auto *widget = qobject_cast<QWidget *>(subjectParent);
         snoopChild = snoopChild ||
                      (widget && widget->testAttribute(Qt::WA_NoChildEventsFromChildren));
         connection = connect(subjectParent, &QObject::destroyed, this,
                              &ParentTracker::lostParent);
      }
      if (snoopChild)
         subject()->installEventFilter(this);
      else {
         Q_ASSERT(subjectParent);
         subject()->removeEventFilter(this);
         subjectParent->installEventFilter(this);
      }
   }

  public:
   explicit ParentTracker(QObject *child) : QObject(child) {
      Q_ASSERT(subject());
      attachToParent();
   }
};

ParentTracker *detachQObjectOwnership(QObject *child) {
   Q_ASSERT(child && (!child->thread() || child->thread() == QThread::currentThread()));
   QObject *parent = child->parent();
   if (!parent) return nullptr;
   if (parent->thread() != child->thread()) return nullptr;
   return new ParentTracker(child);
}

template <class T> void setup(QPointer<QObject> &parent, QPointer<QObject> &child) {
   parent = new T;
   child = new T(static_cast<T*>(parent.data()));
   parent->setObjectName("parent");
   child->setObjectName("child");
   Q_ASSERT(parent && child);
}

int main(int argc, char *argv[]) {
   QApplication app(argc, argv);
   QPointer<QObject> parent, child, tracker;

   // parent-child ownership
   setup<QObject>(parent, child);
   delete parent;
   Q_ASSERT(!parent && !child);

   // parent-child without ownership
   setup<QObject>(parent, child);
   tracker = detachQObjectOwnership(child);
   delete parent;
   Q_ASSERT(!parent && child && tracker);
   delete child;
   Q_ASSERT(!parent && !child && !tracker);

   return 0; // below is TBD
   // parent-child without ownership, parent is mute to child events
   setup<QWidget>(parent, child);
   static_cast<QWidget*>(parent.data())->setAttribute(Qt::WA_NoChildEventsFromChildren);
   tracker = detachQObjectOwnership(child);

   delete parent;
   Q_ASSERT(!parent && child && tracker);
   delete child;
   Q_ASSERT(!parent && !child && !tracker);

   // parent-child without ownership, child doesn't inform parent
   setup<QWidget>(parent, child);
   static_cast<QWidget*>(parent.data())->setAttribute(Qt::WA_NoChildEventsForParent);
   tracker = detachQObjectOwnership(child);
   delete parent;
   Q_ASSERT(!parent && child && tracker);
   delete child;
   Q_ASSERT(!parent && !child && !tracker);
}
