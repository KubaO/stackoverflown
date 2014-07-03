#include <QApplication>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QPixmap>
#include <QMenu>
#include <QList>
#include <QPointer>

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
    Note(TrayMenu *trayMenu, QWidget *parent = 0);
    ~Note();
private:
    QScopedPointer<Ui::Note> m_ui;
    QPointer<TrayMenu> m_traymenu;
};

// TrayMenu.h

class Note;
class TrayMenu : public QSystemTrayIcon
{
public:
    TrayMenu();
    ~TrayMenu();
    void createMainContextMenu();
    void newNote();
private:
    QSystemTrayIcon mainIcon;
    QMenu mainContextMenu;
    QList<QSharedPointer<Note> > noteList;
};

// TrayMenu.cpp

TrayMenu::TrayMenu(){
    mainIcon.setIcon(QIcon(QPixmap("C:\\program.png")));
    mainIcon.setVisible(true);
    mainIcon.show();

    createMainContextMenu();
}

TrayMenu::~TrayMenu() {}

void TrayMenu::newNote() {
    QSharedPointer<Note> note(new Note(this));
    noteList << note;
}

void TrayMenu::createMainContextMenu(){
    QAction *actionNewNote = mainContextMenu.addAction("Neue Notiz");
    mainContextMenu.addSeparator();
    QAction *actionExitProgram = mainContextMenu.addAction("Programm beenden");

    actionNewNote->setIcon(QIcon("C:\\new.ico"));
    actionNewNote->setIconVisibleInMenu(true);

    QObject::connect(actionNewNote, &QAction::triggered, this, &TrayMenu::newNote);
    QObject::connect(actionExitProgram, &QAction::triggered, qApp, &QCoreApplication::quit);

    mainIcon.setContextMenu(&mainContextMenu);
}

// Note.cpp

Note::Note(TrayMenu *trayMenu, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::Note),
    m_traymenu(trayMenu)
{
    m_ui->setupUi(this);
}

Note::~Note() {}

// main.cpp

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   TrayMenu menu;
   return app.exec();
}

#include "main.moc"
