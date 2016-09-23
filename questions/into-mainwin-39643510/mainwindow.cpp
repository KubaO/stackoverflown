//### mainwindow.cpp
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
   for(auto edit : edits) edit->setReadOnly(true);
   layout.addRow("Name:", &lineEditName);
   layout.addRow("Gender:", &lineEditGender);
   layout.addRow("Region:", &lineEditRegion);
   layout.addRow(&button);
   connect(&button, &QPushButton::clicked, this, &MainWindow::request);
}

void MainWindow::setFields(const QString & name, const QString & gender, const QString & region) {
   setState(Normal);
   lineEditName.setText(name);
   lineEditGender.setText(gender);
   lineEditRegion.setText(region);
}

void MainWindow::setState(MainWindow::State state) {
   if (state == Normal) {
      for (auto edit : edits) edit->setEnabled(true);
      button.setEnabled(true);
   }
   else if (state == Loading) {
      for (auto edit : edits) edit->setEnabled(false);
      button.setEnabled(false);
   }
   else if (state == Error) {
      for (auto edit : edits) edit->setText("Error...");
      button.setEnabled(true);
   }
}
