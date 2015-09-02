#include <QApplication>
#include <QMessageBox>
#include <QDebug>

int main(int argc, char** argv)
{
  QApplication app(argc, argv);

  QMessageBox box;
  box.setText("short text");
  box.setWindowTitle("looooooooooooooooong text");
  box.show();
  box.setMinimumSize(qMax(box.minimumWidth(), 800), box.minimumHeight());

  return app.exec();
}
