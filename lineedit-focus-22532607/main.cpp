#include <QApplication>
#include <QLineEdit>
#include <QFormLayout>
#include <QMetaObject>

// Note: A helpful implementation of
// QDebug operator<<(QDebug str, const QEvent * ev)
// is given in http://stackoverflow.com/q/22535469/1329652

/// Returns a cursor to zero position on a QLineEdit on focus-in.
class ReturnOnFocus : public QObject {
   Q_OBJECT
   /// Catches FocusIn events on the target line edit, and appends a call
   /// to resetCursor at the end of the event queue.
   bool eventFilter(QObject * obj, QEvent * ev) {
      QLineEdit * w = qobject_cast<QLineEdit*>(obj);
      // w is nullptr if the object isn't a QLineEdit
      if (w && ev->type() == QEvent::FocusIn) {
         QMetaObject::invokeMethod(this, "resetCursor",
                                   Qt::QueuedConnection, Q_ARG(QWidget*, w));
      }
      // A base QObject is free to be an event filter itself
      return QObject::eventFilter(obj, ev);
   }
   // Q_INVOKABLE is invokable, but is not a slot
   /// Resets the cursor position of a given widget.
   /// The widget must be a line edit.
   Q_INVOKABLE void resetCursor(QWidget * w) {
      static_cast<QLineEdit*>(w)->setCursorPosition(0);
   }
public:
   ReturnOnFocus(QObject * parent = 0) : QObject(parent) {}
   /// Installs the reset functionality on a given line edit
   void installOn(QLineEdit * ed) { ed->installEventFilter(this); }
};

class Ui : public QWidget {
   QFormLayout m_layout;
   QLineEdit m_maskedLine, m_line;
   ReturnOnFocus m_return;
public:
   Ui() : m_layout(this) {
      m_layout.addRow(&m_maskedLine);
      m_layout.addRow(&m_line);
      m_maskedLine.setInputMask("NNNN-NNNN-NNNN-NNNN");
      m_return.installOn(&m_maskedLine);
   }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   Ui ui;
   ui.show();
   return a.exec();
}

#include "main.moc"
