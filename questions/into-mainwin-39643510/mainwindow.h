//### mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

class MainWindow : public QWidget {
  Q_OBJECT
  QFormLayout layout{this};
  QLineEdit lineEditName;
  QLineEdit lineEditGender;
  QLineEdit lineEditRegion;
  QPushButton button{"Get Name"};
  QLineEdit * edits[3] = {&lineEditName, &lineEditGender, &lineEditRegion};
public:
  enum State { Normal, Loading, Error };
  explicit MainWindow(QWidget * parent = nullptr);
  Q_SLOT void setFields(const QString & name, const QString & gender, const QString & region);
  Q_SLOT void setState(State);
  Q_SIGNAL void request();
};

#endif // MAINWINDOW_H
