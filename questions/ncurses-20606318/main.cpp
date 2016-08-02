// https://github.com/KubaO/stackoverflown/tree/master/questions/ncurses-20606318
#include <QtCore>
#include <ncurses.h>

class Worker : public QObject
{
   Q_OBJECT
   QSocketNotifier m_notifier{0, QSocketNotifier::Read, this};
   QBasicTimer m_timer;
   Q_SLOT void readyRead() {
      // It's OK to call this with no data available to be read.
      int c;
      while ((c = getch()) != ERR) {
         printw("%c", (char)(c <= 255 ? c : '?'));
         if (c == 'q' || c == 'Q') qApp->quit();
      }
   }
   void timerEvent(QTimerEvent * ev) {
      if (ev->timerId() != m_timer.timerId()) return;
      printw("*");
      refresh();
   }
public:
   Worker(QObject * parent = 0) : QObject(parent) {
      connect(&m_notifier, SIGNAL(activated(int)), SLOT(readyRead()));
      readyRead(); // data might be already available without notification
      m_timer.start(1000, this);
   }
};

int main(int argc, char *argv[])
{
   QCoreApplication a{argc, argv};
   Worker w;
   auto win = initscr();
   clear();
   cbreak(); // all input is available immediately
   noecho(); // no echo
   printw("Press <q> to quit\n");
   keypad(win, true); // special keys are interpreted and returned as single int from getch()
   nodelay(win, true); // getch() is a non-blocking call
   auto rc = a.exec();
   endwin();
   return rc;
}

#include "main.moc"
