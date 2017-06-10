// https://github.com/KubaO/stackoverflown/tree/master/questions/thread-progress-future-44445248
#include <QtConcurrent>
#include <QtWidgets>
#include <exception>
#include <functional>

struct FileOpenException : std::exception {};
struct FileReadException : std::exception {};
struct Model {};
struct XMLParser {
   XMLParser(Model &) {}
   void read(const QString &) {
      static int outcome;
      QThread::sleep(3);
      switch (outcome++ % 3) {
      case 0: return;
      case 1: throw FileOpenException();
      case 2: throw FileReadException();
      }
   }
};

using Job = std::function<void()>;
Q_DECLARE_METATYPE(Job)

class StudentAbsenceTable : public QMainWindow {
   Q_OBJECT
   QStatusBar m_statusBar;
   QProgressBar m_progress;
   QPushButton m_start{"Start Concurrent Task"};
   Model m_model;
   bool m_documentModified = {};
public:
   StudentAbsenceTable() {
      qRegisterMetaType<Job>();
      m_statusBar.addPermanentWidget(&m_progress);
      m_progress.setMinimum(0);
      m_progress.setMaximum(0);
      m_progress.setMaximumWidth(150);
      m_progress.hide();
      setStatusBar(&m_statusBar);
      setCentralWidget(&m_start);
      connect(&m_start, &QPushButton::clicked, this, [this]{
         m_start.setEnabled(false);
         QtConcurrent::run(this, &StudentAbsenceTable::loadFile);
      });
      connect(this, &StudentAbsenceTable::reqGui, this, [this](const Job & job){
         job();
      });
   }
private:
   bool loadFile() {
      reqGui([=]{ m_progress.show(); });
      auto fileName = QStringLiteral("/media/bsuir/data.xml");
      auto xmlParser = XMLParser(m_model);
      try {
         xmlParser.read(fileName);
         reqGui([=]{
            setCurrentFileName(fileName);
            statusBar()->showMessage(tr("Файл загружен"), 2000);
            m_documentModified = false;
         });
      }
      catch(FileOpenException&) {
         reqGui([=]{
            QMessageBox::warning(this, "Ошибка!", "Ошибка открытия файла!", QMessageBox::Ok);
            statusBar()->showMessage(tr("Загрузка отменена"), 2000);
         });
      }
      catch(FileReadException&) {
         reqGui([=]{
            QMessageBox::warning(this, "Ошибка!", "Ошибка чтения файла!", QMessageBox::Ok);
            statusBar()->showMessage(tr("Загрузка отменена"), 2000);
         });
      }
      reqGui([=]{ m_progress.hide(); m_start.setEnabled(true); });
      return false;
   }
   Q_SIGNAL void reqGui(const Job &);
   void setCurrentFileName(const QString &) {}
};

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   StudentAbsenceTable ui;
   ui.setMinimumSize(350, 350);
   ui.show();
   return app.exec();
}
#include "main.moc"
