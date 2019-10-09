// https://github.com/KubaO/stackoverflown/tree/master/questions/mainwindow-crash-double-delete-58305305
#include <QtWidgets>

class Window : public QMainWindow {
   QWidget central;
   QGridLayout layout{&central};
   QLabel label{"Hello, World!"};
   QCheckBox crash{"Crash on exit"};
public:
   static bool alive;
   Window() {
      alive = true;
      layout.addWidget(&label, 0, 0);
      layout.addWidget(&crash, 1, 0);
      setCentralWidget(&central);
      connect(&crash, &QCheckBox::toggled, this, [this](){
         setAttribute(Qt::WA_DeleteOnClose, crash.isChecked());
      });
   }
   ~Window() {
      qDebug() << __FUNCTION__;
      alive = false;
   }
};
bool Window::alive;

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   int rc;
   {
      Window w;
      w.show();
      rc = a.exec();
      Q_ASSERT(w.alive);
      w.setWindowTitle("A New Title Awaits");
      qDebug() << "We're past w.setWindowTitle()";
   }
   return rc;
}
