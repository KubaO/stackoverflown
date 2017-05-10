// https://github.com/KubaO/stackoverflown/tree/master/questions/event-discard-43885834
#include <QtWidgets>

void wipeEvent(QEvent * event) {
   struct Helper : QEvent {
      static void wipe(QEvent * e) {
         static_cast<Helper*>(e)->t = QEvent::None;
      }
   };
   Helper::wipe(event);
}

void wipeEvent2(QEvent *event) {
   event->~QEvent(); // OK since the destructor is virtual.
   new (event) QEvent(QEvent::None);
}

class ChildWidget : public QWidget {
   Q_OBJECT
   QHBoxLayout m_layout{this};
   QLabel m_label{"ChildLabel"};
public:
   ChildWidget() {
      setAcceptDrops(true);
      m_layout.addWidget(&m_label);
   }
   void dragEnterEvent(QDragEnterEvent *event) override {
      qDebug() << "Child";
      while (auto mimeData=event->mimeData()) {
         auto url = QUrl(mimeData->text());
         if (!url.isValid()) break;
         if (!url.isLocalFile()) break;
         auto filename = url.fileName();
         if (!filename.endsWith(".txt")) break;
         // ChildWidget can only process txt files.
         qDebug() << url.fileName();
         return event->acceptProposedAction();
      }
      wipeEvent(event);
   }
};

class ParentWidget : public QWidget {
   Q_OBJECT
   QHBoxLayout m_layout{this};
   QLabel m_label{"ParentLabel"};
   ChildWidget m_child;
public:
   ParentWidget() {
      setAcceptDrops(true);
      m_layout.addWidget(&m_label);
      m_layout.addWidget(&m_child);
   }
   void dragEnterEvent(QDragEnterEvent *event) override {
      qDebug() << "Parent";
      event->acceptProposedAction();
   }
};

int main(int argc, char** args) {
   QApplication app{argc, args};
   ParentWidget widget;
   widget.show();
   app.exec();
}
#include "main.moc"
