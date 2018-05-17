// https://github.com/KubaO/stackoverflown/tree/master/questions/note-tray-21753641
#include <QtWidgets>
#include <list>

// Note.h

namespace Ui{
class Note {
public:
   void setupUi(QWidget *) {} // dummy for sscce.org
};
}

class TrayMenu;
class Note : public QWidget
{
   Q_OBJECT
public:
   Note(TrayMenu *trayMenu, QWidget *parent = {});
private:
   Ui::Note m_ui;
   QPointer<TrayMenu> m_traymenu;
};

// TrayMenu.h

class Note;
class TrayMenu : public QObject {
   Q_OBJECT
public:
   TrayMenu();
   void createMainContextMenu();
   void newNote();
private:
   QSystemTrayIcon m_mainIcon;
   QMenu m_mainContextMenu;
   std::list<Note> m_notes;
};

// TrayMenu.cpp

template <int N> auto decode64(const char (&arg)[N], int rows) {
   auto const raw = QByteArray::fromBase64(QByteArray::fromRawData(arg, N-1));
   QImage img((const quint8 *)raw.data(), rows, rows, raw.size()/rows, QImage::Format_MonoLSB);
   img = std::move(img).convertToFormat(QImage::Format_Indexed8);
   img.setColor(1, qRgba(0, 0, 0, 0)); // make transparent
   return img;
}

// convert baseline_language_black_18dp.png -flatten -negate -monochrome mono:-|base64 -b80
static const char language_d64[] =
      "/////w//////D/////8P/z/A/w//DwD+D/8BAPwP/wEA+A9/IEbgDz8cjuEPPxyPww8fHg+HDw+PHw8P"
      "DwAAAA8PAAAADw8AAAAPx8c/Pg7Hzz8+DsfHHz4Ox4c/Pg7Hxz8/DsfHPz4ODwAAAA4PAAAADw8AAAAP"
      "H48fjw8fHg+HDz8cj4MPPxiH4Q9/IMbgD/8AAPAP/wMA/A//DwD/D/8/4P8P/////w//////D/////8P";
static const auto language_icon = decode64(language_d64, 36);

// convert baseline_note_add_black_18dp.png -flatten -negate -monochrome mono:-|base64 -b80
static const char note_add_d64[] =
      "/////w//////D/////8PfwDA/w8/AMD/Dz8AAP8PPwAY/w8/ADD8Dz8AePwPPwDw8A8/APjhDz8A8OMP"
      "PwDwxw8/AJDCDz8AAMAPPwAAwA8/AATADz8AD8APPwAGwA8/AA/ADz8ABsAPP/D/wA8/8P/ADz/w/8AP"
      "PwAOwA8/AAfADz8ADsAPPwAGwA8/AAbADz8AAMAPPwAAwA8/AADAD38AAOAP/////w//////D/////8P";
static const auto note_add_icon = decode64(note_add_d64, 36);

TrayMenu::TrayMenu() {
   m_mainIcon.setIcon(QPixmap::fromImage(language_icon));
   m_mainIcon.setVisible(true);
   m_mainIcon.show();
   createMainContextMenu();
}

void TrayMenu::newNote() {
   m_notes.emplace_back(this);
   m_notes.back().show();
}

void TrayMenu::createMainContextMenu() {
   auto *actionNewNote = m_mainContextMenu.addAction("Neue Notiz");
   m_mainContextMenu.addSeparator();
   auto *actionExitProgram = m_mainContextMenu.addAction("Programm beenden");

   actionNewNote->setIcon(QPixmap::fromImage(note_add_icon));
   actionNewNote->setIconVisibleInMenu(true);

   QObject::connect(actionNewNote, &QAction::triggered, this, &TrayMenu::newNote);
   QObject::connect(actionExitProgram, &QAction::triggered, QCoreApplication::quit);

   m_mainIcon.setContextMenu(&m_mainContextMenu);
}

// Note.cpp

Note::Note(TrayMenu *trayMenu, QWidget *parent) :
   QWidget(parent),
   m_traymenu(trayMenu)
{
   m_ui.setupUi(this);
}

// main.cpp

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   TrayMenu menu;
   return app.exec();
}
#include "main.moc"
