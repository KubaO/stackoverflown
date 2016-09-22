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
  explicit MainWindow(QWidget * parent = nullptr);
  void setFields(const QString & name, const QString & gender, const QString & region);
  void setAllFields(const QString & value);
  Q_SIGNAL void request();
  Q_SLOT void disableButton() { button.setDisabled(true); }
  Q_SLOT void enableButton() { button.setEnabled(true); }
};

#endif // MAINWINDOW_H
