// main.cpp
#include <QApplication>
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QMetaObject>
#include <QDebug>

class MyUi : public QWidget {
   Q_OBJECT
   QBoxLayout m_layout;
   QLabel m_label;
   QPushButton m_button;
   QMessageBox m_warning;
   Q_SLOT void on_button_clicked() {
      m_warning.show();
   }
   Q_SLOT void on_warning_finished(int rc) {
      // The `finished()` signal is emitted with a
      // QDialogButtonBox::StandardButton value - the same that would
      // be retuned by QMessageBox::exec().
      // A QMessageBox does *not*  accept the dialog,
      // so we can't simply use the `accepted` signal.
      if (rc != QDialogButtonBox::Yes) return;
      m_label.setText(m_label.text() + "*v*");
   }
public:
   MyUi(QWidget * parent = 0) :
      QWidget(parent),
      m_layout(QBoxLayout::TopToBottom, this),
      m_button("Change Message"),
      m_warning(QMessageBox::Warning, "Message Change",
                "The message will change.",
                QMessageBox::Yes | QMessageBox::No,
                this)
   {
      m_button.setObjectName("button");
      m_warning.setObjectName("warning");
      m_warning.setWindowModality(Qt::WindowModal);
      m_warning.setInformativeText(
               "Are you sure you want the message to change?");
      m_layout.addWidget(&m_label);
      m_layout.addWidget(&m_button);
      QMetaObject::connectSlotsByName(this);
   }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   MyUi ui;
   ui.show();
   return a.exec();
}

#include "main.moc"
