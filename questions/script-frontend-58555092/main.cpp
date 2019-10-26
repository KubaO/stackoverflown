// https://github.com/KubaO/stackoverflown/tree/master/questions/script-frontend-58555092
#include <QtWidgets>
#include <initializer_list>

class MainWindow : public QWidget {
   Q_OBJECT
   QProcess process;

   QGridLayout layout{this};
   QPushButton selectSource{tr("Select HiSuite Backup Folder")};
   QLineEdit source;
   QPushButton selectDestination{tr("Select Destination Folder")};
   QLineEdit destination;
   QLabel instruction{tr(
       "Type here the password given during the creation of "
       "the backup of the mobile device with Huawei HiSuite.")};
   QLineEdit password;
   QPushButton decrypt{tr("Decrypt HiSuite Backup")};
   QTextBrowser output;

   QFileSystemModel fsModel;
   QCompleter fsCompleter{&fsModel};

   QTextCharFormat statusFormat, commandFormat, stdoutFormat, stderrFormat;

   void setupUi();
   void doDecrypt();
   void onStateChanged(QProcess::ProcessState state);

  public:
   explicit MainWindow(QWidget *parent = nullptr);
};

void MainWindow::setupUi() {
   setWindowTitle(tr("Huawei HiSuite Backup Decrypter"));
   QFont boldFont;
   boldFont.setBold(true);
   decrypt.setFont(boldFont);
   instruction.setWordWrap(true);

   commandFormat.setForeground(QColor(Qt::green).darker());
   statusFormat.setForeground(QColor(Qt::blue));
   stderrFormat.setForeground(QColor(Qt::red).darker());

   int row = 0;
   for (auto *widget : std::initializer_list<QWidget *>{
            &selectSource, &source, &selectDestination, &destination, &instruction,
            &password, &decrypt, &output}) {
      layout.addWidget(widget, row++, 0);
   }

   for (auto *button : {&selectSource, &selectDestination, &decrypt})
      button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

   fsModel.setRootPath("");
   fsModel.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
   fsCompleter.setCompletionMode(QCompleter::PopupCompletion);
   source.setCompleter(&fsCompleter);
   destination.setCompleter(&fsCompleter);
}

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
   setupUi();

   connect(&selectSource, &QPushButton::clicked, [this] {
      auto dir = QFileDialog::getExistingDirectory(
          this, tr("Choose HiSuite Backup Folder"), QDir::homePath(),
          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
      source.setText(dir);
   });

   connect(&selectDestination, &QPushButton::clicked, [this] {
      auto dir = QFileDialog::getExistingDirectory(
          this, tr("Choose Destination Folder"), QDir::homePath(),
          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
      destination.setText(dir);
   });

   connect(&decrypt, &QPushButton::clicked, this, &MainWindow::doDecrypt);

   connect(&process, &QProcess::stateChanged, this, &MainWindow::onStateChanged);
   connect(&process, &QProcess::readyReadStandardOutput, [this] {
      output.setCurrentCharFormat(stdoutFormat);
      output.append(process.readAllStandardOutput());
   });
   connect(&process, &QProcess::readyReadStandardError, [this] {
      output.setCurrentCharFormat(stderrFormat);
      output.append(process.readAllStandardError());
   });
}

void MainWindow::onStateChanged(QProcess::ProcessState state) {
   output.setCurrentCharFormat(statusFormat);
   switch (state) {
      case QProcess::Starting:
         output.append(tr("(starting)"));
         break;
      case QProcess::Running:
         output.append(tr("(started)"));
         break;
      case QProcess::NotRunning:
         output.append(tr("(not running)"));
         break;
   }
}

void MainWindow::doDecrypt() {
   output.clear();
   auto password = this->password.text();

   QString scriptFile = QCoreApplication::applicationDirPath() + "/HHBD.py";
   process.setProgram("python");
   process.setArguments({QDir::toNativeSeparators(scriptFile), password,
                         QDir::toNativeSeparators(source.text()),
                         QDir::toNativeSeparators(destination.text())});
   output.setCurrentCharFormat(commandFormat);
   output.setText(
       QStringLiteral("%1 %2").arg(process.program()).arg(process.arguments().join(' ')));
   process.start();
}

int main(int argc, char *argv[]) {
   QApplication a(argc, argv);
   MainWindow w;
   w.show();
   return a.exec();
}
#include "main.moc"
