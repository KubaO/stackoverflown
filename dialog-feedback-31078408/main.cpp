#include <QApplication>
#include <QFileInfo>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QThread>
#include <QTimer>

class Thread : public QThread {
   using QThread::run; // final
public:
   ~Thread() { quit(); wait(); }
};

class Checker : public QObject {
   Q_OBJECT
public:
   Q_SIGNAL void exists(bool, const QString & path);
   Q_SLOT void check(const QString & path) { emit exists(QFileInfo::exists(path), path); }
};

int main(int argc, char *argv[])
{
   bool pathExists = true;
   QApplication app(argc, argv);
   QDialog dialog;
   QLineEdit edit("/");
   QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Close);
   QGridLayout layout(&dialog);
   layout.addWidget(&edit, 0, 0);
   layout.addWidget(&buttons, 1, 0);

   QTimer checkTimer;
   Checker checker;
   Thread checkerThread;
   checker.moveToThread(&checkerThread);
   checkerThread.start();
   checkTimer.setInterval(500);
   checkTimer.setSingleShot(true);

   QObject::connect(&buttons, &QDialogButtonBox::accepted, [&]{
      if (!pathExists) return;
      //...
      dialog.accept();
   });
   QObject::connect(&buttons, &QDialogButtonBox::rejected, [&]{ dialog.reject(); });

   QObject::connect(&edit, &QLineEdit::textChanged, &checker, &Checker::check);
   QObject::connect(&edit, &QLineEdit::textChanged, &checkTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
   QObject::connect(&checkTimer, &QTimer::timeout, [&]{ edit.setStyleSheet("background: yellow"); });

   QObject::connect(&checker, &Checker::exists, &app, [&](bool ok, const QString & path){
      if (path != edit.text()) return; // stale result
      checkTimer.stop();
      edit.setStyleSheet(ok ? "" : "background: red");
      buttons.button(QDialogButtonBox::Ok)->setEnabled(ok);
      pathExists = ok;
   });

   dialog.show();
   return app.exec();
}

#include "main.moc"
