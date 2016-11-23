// https://github.com/KubaO/stackoverflown/tree/master/questions/messagebox-roles-40753898
#include <QtWidgets>

class MessageBoxAdapter : public QObject {
   Q_OBJECT
public:
   MessageBoxAdapter(QObject *parent = nullptr) : QObject(parent) {
      watch(parent);
   }
   void watch(QObject *obj) {
      auto box = qobject_cast<QMessageBox*>(obj);
      if (!box) return;
      connect(box, &QMessageBox::rejected, [=]{
         if (!box->clickedButton()) emit rejected();
      });
      connect(box, &QMessageBox::buttonClicked, [=](QAbstractButton *button){
         auto role = box->buttonRole(button);
         if (role == QMessageBox::AcceptRole) emit accepted();
         else if (role == QMessageBox::RejectRole) emit rejected();
         emit roleClicked(role);
      });
   }
   Q_SIGNAL void accepted();
   Q_SIGNAL void rejected();
   Q_SIGNAL void roleClicked(QMessageBox::ButtonRole role);
};

struct Ui : public QWidget {
   QVBoxLayout layout{this};
   QTextBrowser browser;
   QPushButton button{"Open"};
   MessageBoxAdapter adapter{this};
public:
   Ui() {
      layout.addWidget(&browser);
      layout.addWidget(&button);
      connect(&button, &QPushButton::clicked, this, &Ui::onClicked);
      connect(&adapter, &MessageBoxAdapter::accepted, [=]{ browser.append("accepted"); });
      connect(&adapter, &MessageBoxAdapter::rejected, [=]{ browser.append("rejected"); });
      connect(&adapter, &MessageBoxAdapter::roleClicked, [=](QMessageBox::ButtonRole role){
         browser.append(QStringLiteral("clicked role=%1").arg(role));
      });
   }
   void onClicked() {
      auto box = new QMessageBox{this};
      adapter.watch(box);
      box->setAttribute(Qt::WA_DeleteOnClose);
      box->addButton("OK", QMessageBox::AcceptRole);
      box->addButton("ACTION", QMessageBox::ActionRole);
      box->addButton("CANCEL", QMessageBox::RejectRole);
      box->setIcon(QMessageBox::Question);
      box->setWindowTitle("Hola");
      box->setText("My 1st message.");
      box->show();
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Ui ui;
   ui.show();
   return app.exec();
}
#include "main.moc"
