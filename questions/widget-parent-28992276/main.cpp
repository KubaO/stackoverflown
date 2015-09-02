#if 0
#include <QApplication>
#include <QLabel>

class ParentHacker : private QWidget {
public:
   static void setParent(QWidget * child_, QObject * parent) {
      // The following line invokes undefined behavior
      ParentHacker * child = static_cast<ParentHacker*>(child_);
      Q_ASSERT(child->d_ptr->isWidget);
      child->d_ptr->isWidget = 0;
      child->QObject::setParent(parent);
      child->d_ptr->isWidget = 1;
   }
};

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   QLabel w("Hello!");
   w.setMinimumSize(200, 100);
   w.show();
   ParentHacker::setParent(&w, &app);
   return app.exec();
}

#else
#include <QApplication>
#include <QLabel>

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   QWidget parent;
   QLabel l1("Close me to quit!"), l2("Hello!");
   foreach (QLabel * label, QVector<QLabel*>() << &l1 << &l2) {
      label->setMinimumSize(200, 100);
      label->setParent(&parent);
      label->setWindowFlags(Qt::Window);
      label->setText(QString("%1 Parent: %2.").
                     arg(label->text()).arg((intptr_t)label->parent(), 0, 16));
      label->show();
   }
   l2.setAttribute(Qt::WA_QuitOnClose, false);
   return app.exec();
}

#endif
