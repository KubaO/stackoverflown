#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QDialogButtonBox>

class MyDialog : public QDialog {
   QGridLayout m_layout;
   QDialogButtonBox m_box;
public:
   MyDialog(QWidget * parent = 0) : QDialog(parent), m_layout(this),
      m_box(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
   {
      m_layout.addWidget(&m_box);
      connect(&m_box, SIGNAL(accepted()), SLOT(accept()));
      connect(&m_box, SIGNAL(rejected()), SLOT(reject()));
   }
};

class MyGui : public QWidget {
   Q_OBJECT
   QGridLayout m_layout;
   QLabel m_label;
   QPushButton m_button;
   MyDialog m_dialog;
   Q_SLOT void on_button_clicked() {
      m_dialog.show();
   }
   Q_SLOT void on_dialog_accepted() {
      m_label.setText("The dialog was accepted");
   }
   Q_SLOT void on_dialog_rejected() {
      m_label.setText("The dialog was rejected");
   }
public:
   MyGui() : m_layout(this), m_button("Show Dialog"), m_dialog(this) {
      m_button.setObjectName("button");
      m_dialog.setObjectName("dialog");
      m_layout.addWidget(&m_label);
      m_layout.addWidget(&m_button);
      QMetaObject::connectSlotsByName(this);
   }
};

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   MyGui gui;
   gui.show();
   return app.exec();
}

#include "main.moc"
