// https://github.com/KubaO/stackoverflown/tree/master/questions/messagebox-show-25545652
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif
#include <functional>

int main1(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QMessageBox msg;
   msg.setText("Hello");
   msg.addButton(QMessageBox::Close);
   msg.show();
   return a.exec();
}

// Qt 4 only
struct FunctorSlot : public QObject {
   Q_OBJECT
public:
   std::function<void()> callable;
   template <typename Fun>
   FunctorSlot(Fun && fun, QObject * parent = {}) :
      QObject{parent}, callable{std::forward<Fun>(fun)} {}
   Q_SLOT void call() {
      callable();
   }
};

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);

   QMessageBox msg;
   msg.setText("Continue?");
   msg.addButton(QMessageBox::Yes);
   msg.addButton(QMessageBox::No);
   auto onClick = [&msg]() {
      auto role = msg.buttonRole(msg.clickedButton());
      if (role == QMessageBox::NoRole)
         QApplication::quit();
      if (role == QMessageBox::YesRole) {
         auto label = new QLabel("I'm running");
         label->setAttribute(Qt::WA_DeleteOnClose);
         label->show();
      }
   };
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
   QObject::connect(&msg, &QMessageBox::buttonClicked, onClick);
#else
   QObject::connect(&msg, SIGNAL(buttonClicked(QAbstractButton*)),
                    new FunctorSlot{onClick, &msg}, SLOT(call()));
#endif
   msg.show();
   return app.exec();
}
#include "main.moc"
