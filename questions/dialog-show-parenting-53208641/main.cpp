// https://github.com/KubaO/stackoverflown/tree/master/questions/dialog-show-parenting-53208641
#include <QtWidgets>

class Parent : public QDialog {
   Q_OBJECT
   QVBoxLayout layout{this};
   QDialog child;
   QPushButton cShow{tr("Show child")}, cNonWindow{tr("Renew non-window child")},
       cWindow{tr("Renew window child")};
   Q_SLOT void on_child_accepted() {}
   void reChild(bool makeWindow) {
      child.~QDialog();
      new (&child) QDialog;
      Q_ASSERT(child.isWindow());
      child.setParent(this);
      child.setObjectName("child");
      child.setStyleSheet("QWidget { background: blue }");
      if (makeWindow) {
         child.setWindowFlag(Qt::Dialog);
         Q_ASSERT(child.isWindow());
      } else {
         Q_ASSERT(!child.isWindow());
         child.show();  // The child gets shown when we're shown
      }
      QMetaObject::invokeMethod(this, &Parent::updateChild, Qt::QueuedConnection);
   }
   void updateChild() {
      if (!child.isWindow()) child.move(50, cWindow.y() + cWindow.height() / 2);
      this->update();  // Work around a refresh bug (affects OS X on 5.11 at least)
   }

  public:
   Parent(QWidget *parent = nullptr, Qt::WindowFlags f = {}) : QDialog{parent, f} {
      connect(&cShow, &QPushButton::clicked, [&]() { child.show(); });
      connect(&cNonWindow, &QPushButton::clicked, [&] { reChild(false); });
      connect(&cWindow, &QPushButton::clicked, [&] { reChild(true); });
      for (auto *w : {&cShow, &cNonWindow, &cWindow}) layout.addWidget(w);
      cNonWindow.click();
      QMetaObject::connectSlotsByName(this);
   }
};

int main(int argc, char *argv[]) {
   QApplication a(argc, argv);
   Parent w;
   w.show();
   return a.exec();
}
#include "main.moc"
