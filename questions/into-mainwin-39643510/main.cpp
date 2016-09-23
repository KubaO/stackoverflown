//### main.cpp
// https://github.com/KubaO/stackoverflown/tree/master/questions/into-mainwin-39643510
#include "mainwindow.h"
#include "controller.h"

int main(int argc, char *argv[])
{
   QApplication app{argc, argv};
   MainWindow ui;
   Controller ctl;
   QTimer timer;
   timer.start(5*1000);
   QObject::connect(&timer, &QTimer::timeout, &ctl, &Controller::get);

   QObject::connect(&ctl, &Controller::busy, &ui, [&]{ ui.setState(MainWindow::Loading); });
   QObject::connect(&ui, &MainWindow::request, &ctl, &Controller::get);
   QObject::connect(&ctl, &Controller::error, &ui, [&]{ ui.setState(MainWindow::Error); });
   QObject::connect(&ctl, &Controller::values, &ui, &MainWindow::setFields);
   ui.show();
   return app.exec();
}
