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
   lineEditName.setText(name);
   lineEditGender.setText(gender);
   lineEditRegion.setText(region);
}

void MainWindow::setAllFields(const QString & value) {
   for (auto edit : edits) edit->setText(value);
}
