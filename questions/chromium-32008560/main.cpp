//main.cpp
#include <QtWebEngineWidgets>
#include <QApplication>

int main(int argc, char ** argv)
{
   QApplication a(argc, argv);
   QWebEngineView view;
   view.load(QUrl("http://google.com"));
   view.showFullScreen();
   return a.exec();
}
