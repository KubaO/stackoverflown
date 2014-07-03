#include <QApplication>
#include <QTextEdit>
#include <QTextBrowser>
#include <QHBoxLayout>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QWidget window;
   QHBoxLayout layout(&window);
   QTextEdit edit;
   QTextBrowser browser;
   layout.addWidget(&edit);
   layout.addWidget(&browser);
   browser.setDocument(edit.document());
   window.show();
   return a.exec();
}
