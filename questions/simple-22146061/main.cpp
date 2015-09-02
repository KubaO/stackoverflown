#include <QApplication>
#include <QMessageBox>

int main(int argc, char ** argv)
{
  // It is an error not to have an instance of QApplication.
  // This implies that having an instance of QCoreApplication and QGuiApplication
  // is also an error.
  QApplication app(argc, argv);
  QMessageBox::information(NULL, "Get This!", "Something's going on");
}

