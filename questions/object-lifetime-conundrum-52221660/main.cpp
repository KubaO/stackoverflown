// https://github.com/KubaO/stackoverflown/tree/master/questions/object-lifetime-conundrum-52221660
#define QT_FORCE_ASSERTS
#include <QtCore>
#include <memory>

static const char kEnter[8] = "(enter)", kLeave[8] = "(leave)", kBlip[6] = "*ran*";
class Scope {
   Q_DISABLE_COPY(Scope)
   QObject *const obj;
   QByteArray const property;
   static QObject &proprietor() {
      static QObject p;
      return p;
   }
   static void indicate(const char *prop, QObject *obj, const char *event) {
      auto dbg = QDebug(QtMsgType::QtDebugMsg) << prop;
      if (!obj->objectName().isEmpty()) dbg << obj->objectName();
      dbg << event;
      Q_ASSERT(isIn(prop) == (event == kLeave));
      proprietor().setProperty(prop, event == kEnter);
   }

  public:
   enum When { Out = 1, InOut = 2 } const when;
   Scope(const char *p, QObject *o, When w = InOut) : obj(o), property(p), when(w) {
      if (when == InOut) in();
   }
   void in() { indicate(property, obj, kEnter); }
   ~Scope() { indicate(property, obj, kLeave); }
   struct GoIn {
      GoIn(Scope &scope) { scope.in(); }
   };
   static void blip(const char *prop, QObject *o) { Scope::indicate(prop, o, kBlip); }
   static void shred(const char *prop) { proprietor().setProperty(prop, {}); }
   static bool had(const char *prop) { return proprietor().property(prop).isValid(); }
   static bool isIn(const char *prop) { return proprietor().property(prop).toBool(); }
};

class Part : public QObject {
   Q_OBJECT
  public:
   Part(QObject *parent = nullptr) : QObject(parent) {
      Scope::blip("part_constructor_body", this);
      Q_ASSERT(!Scope::isIn("Foo_Type"));
   }
   ~Part() override {
      Scope scope("part_destructor_body", this);
      emit signal();
   }
   Q_SIGNAL void signal();
};

class Foo : public QObject {
   Q_OBJECT
   Scope scope{"Foo_Type", this, Scope::Out};
   Part part1{this};  // a child owned by value - bravo! - the lowest overhead approach
   Part part2;        // ditto, made a child in the constructor's initializer list
   Part part3;        // fine, but not a child of Foo, and thus Foo's `moveToThread()`
                      // will almost always set Part up for undefined behavior
   // the below all have the overhead of an extra indirection - an entirely gratuitous one
   std::unique_ptr<Part> part1b{new Part(this)};
   std::unique_ptr<Part> part2b;
   std::unique_ptr<Part> part3b{new Part};
   // and the snafu
   Part *part4{new Part(this)};
   Scope::GoIn into{scope};

  public:
   Foo(Qt::ConnectionType type = Qt::AutoConnection)
       : part2(this), part2b(new Part(this)) {
      Scope scope("foo_constructor_body", this, Scope::InOut);
      part4->setObjectName("part4");
      for (auto *p :
           {&part1, &part2, &part3, part1b.get(), part2b.get(), part3b.get(), part4})
         connect(p, SIGNAL(signal()), this, SLOT(slot()), type);
   }
   ~Foo() override { Scope::blip("foo_destructor_body", this); }

   Q_SLOT void slot() {
      Scope::blip("foo_slot_body", sender());
      Q_ASSERT(qobject_cast<Foo *>(this));
      Q_ASSERT(Scope::isIn("Foo_Type"));  // equivalent to the foregoing assert
   }
};

int main(int argc, char *argv[]) {
   qDebug() << "*** before the application object is created";
   Foo{};
   QCoreApplication app(argc, argv);
   qDebug() << "*** after the application object is created";
   Foo{};
   qDebug() << "*** with queued connections" << Qt::QueuedConnection;
   {
      Q_ASSERT(Scope::had("foo_slot_body"));
      Scope::shred("foo_slot_body");
      Foo foo3(Qt::QueuedConnection);  // check with queued connections
      QTimer::singleShot(1, &app, SLOT(quit()));
      app.exec();
      Q_ASSERT(!Scope::had("foo_slot_body"));
   }
}
#include "main.moc"
