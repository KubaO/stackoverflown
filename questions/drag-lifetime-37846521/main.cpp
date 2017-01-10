// https://github.com/KubaO/stackoverflown/tree/master/questions/drag-lifetime-37846521
#include <QtWidgets>

template <typename F>
static void postToThread(F && fun, QObject * obj = qApp) {
   QObject src;
   QObject::connect(&src, &QObject::destroyed, obj, std::forward<F>(fun),
                    Qt::QueuedConnection);
}

struct MyListWidget : QListWidget {
   Q_SIGNAL void dragStarted();
   Q_SIGNAL void dragStopped();
   MyListWidget() {
      setDragEnabled(true);
      addItem("item1");
      addItem("item2");
   }
   void startDrag(Qt::DropActions supportedActions) override {
      postToThread([this]{
         auto drag = findChild<QDrag*>();
         if (drag) {
            emit dragStarted();
            connect(drag, &QObject::destroyed, this, &MyListWidget::dragStopped);
         }
      }, this);
      QListWidget::startDrag(supportedActions); // reenters the event loop
   }
   Q_OBJECT
};

int main(int argc, char **argv) {
   QApplication app(argc, argv);
   QWidget gui;
   QVBoxLayout layout(&gui);
   MyListWidget list;
   QLabel label;
   layout.addWidget(&list);
   layout.addWidget(&label);
   QObject::connect(&list, &MyListWidget::dragStarted, [&]{ label.setText("Drag Active"); });
   QObject::connect(&list, &MyListWidget::dragStopped, &label, &QLabel::clear);
   gui.show();
   return app.exec();
}
#include "main.moc"
