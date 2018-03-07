// https://github.com/KubaO/stackoverflown/tree/master/questions/async-open-load-49052157
#include <QtWidgets>
#include <QtConcurrent>

struct MenuEntry {
   QAction action;
   enum Options { None = 0, Quit = 1, Separator = 2} options;
   MenuEntry(const QString & text, const QKeySequence & shortcut,
             std::function<void()> && slot, Options options = None) :
      action(text), options(options) {
      action.setShortcut(shortcut);
      action.setMenuRole((options & MenuEntry::Quit) ? QAction::QuitRole : QAction::NoRole);
      QObject::connect(&action, &QAction::triggered, std::move(slot));
   }
};

class Notepad : public QMainWindow
{
   Q_OBJECT
public:
   Notepad();

private:
   QMenu m_fileMenu{tr("&File")};
   MenuEntry m_fileActions[3] = {
      { tr("&Open"), QKeySequence::Open, [this]{ open(); }},
      { tr("&Save As..."), QKeySequence::SaveAs, [this]{ save(); }, MenuEntry::Separator},
      { tr("&Quit"), QKeySequence::Quit, []{ qApp->quit(); }, MenuEntry::Quit }
   };

   QMessageBox m_message{this};
   QFileDialog m_dialog{this};
   QTextEdit m_edit;
   Q_SLOT void open();
   Q_SLOT void save();

   template <class F> void onGui(F &&slot) {
      QTimer::singleShot(0, Qt::PreciseTimer, this, std::move(slot));
   }
   void critical(const QString & title, const QString & text);
   template <class F>
   void onFile(QFileDialog::AcceptMode mode, const QString & title, F &&slot) {
      auto context = new QObject{this};
      connect(&m_dialog, &QFileDialog::finished, context, [this, slot, context]{
         if (!m_dialog.selectedFiles().isEmpty())
            slot(m_dialog.selectedFiles().first());
         delete context;
      });
      dialogOpen(mode, title);
   }
   void dialogOpen(QFileDialog::AcceptMode mode, const QString & title);
};

void Notepad::critical(const QString & title, const QString & text) {
   onGui([this, title, text]{
      m_message.setIcon(QMessageBox::Critical);
      m_message.setWindowTitle(title);
      m_message.setText(text);
      m_message.show();
   });
}

void Notepad::dialogOpen(QFileDialog::AcceptMode mode, const QString & title) {
   m_dialog.setAcceptMode(mode);
   m_dialog.setFileMode((mode == QFileDialog::AcceptOpen) ?
                           QFileDialog::ExistingFile : QFileDialog::AnyFile);
   m_dialog.setWindowTitle(title);
   m_dialog.show();
}

Notepad::Notepad() {
   for (auto &a : m_fileActions) {
      m_fileMenu.addAction(&a.action);
      if (a.options & MenuEntry::Separator) m_fileMenu.addSeparator();
   }
   menuBar()->addMenu(&m_fileMenu);

   setCentralWidget(&m_edit);
   m_message.setModal(true);
   m_message.setIcon(QMessageBox::Critical);
   m_dialog.setNameFilter(tr("Text Files (*.txt);;C++ Files (*.cpp *.h)"));
   m_dialog.setWindowModality(Qt::WindowModal);

   setWindowTitle(tr("Notepad"));
}

void Notepad::open() {
   onFile(QFileDialog::AcceptOpen, tr("Open File"), [this](const QString &fileName){
      if (fileName.isEmpty()) return;
      QtConcurrent::run([this, fileName]{
         QFile file(fileName);
         if (!file.open(QIODevice::ReadOnly))
            return critical(tr("Error"), tr("Could not open file."));
         QTextStream in(&file);
         auto text = in.readAll();
         if (in.status() == QTextStream::Ok) onGui([this, text]{
            m_edit.setText(text);
         }); else
            return critical(tr("Error"), tr("Could not read the file."));
      });
   });
}

void Notepad::save() {
   onFile(QFileDialog::AcceptSave, tr("Save File"), [this](const QString &fileName){
      if (fileName.isEmpty()) return;
      auto text = m_edit.toPlainText();
      QtConcurrent::run([this, fileName, text]{
         QSaveFile file(fileName);
         if (!file.open(QIODevice::WriteOnly))
            return critical(tr("Error"), tr("Could not open file."));
         QTextStream out(&file);
         out << text;
         if (out.status() != QTextStream::Ok || !file.commit())
            return critical(tr("Error"), tr("Could not write to file."));
      });
   });
}

int main(int argc, char **argv)
{
   QApplication app(argc, argv);
   return Notepad().show(), app.exec();
}

#include "main.moc"
