//### main.cpp
// https://github.com/KubaO/stackoverflown/tree/master/questions/into-mainwin-39643510
#include "mainwindow.h"
#include "controller.h"

int main(int argc, char *argv[])
{
   QApplication app{argc, argv};
   MainWindow ui;
   Controller ctl;

   QObject::connect(&ui, &MainWindow::request, [&]{
      ui.setAllFields("Loading...");
      ui.disableButton();
      ctl.get();
   });
   QObject::connect(&ctl, &Controller::error, &ui, [&]{
      ui.setAllFields("Error...");
      ui.enableButton();
   });
   QObject::connect(&ctl, &Controller::values, &ui, &MainWindow::setFields);
   QObject::connect(&ctl, &Controller::values, &ui, &MainWindow::enableButton);
   ui.show();
   return app.exec();
}
