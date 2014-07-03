#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLineEdit>
#include <QIntValidator>
#include <QFormLayout>
#include <QSet>

static void styleSheetSet(QWidget * w, const QString & what)
{
   if (what.isEmpty()) return;
   QString token = "/*>*/" + what + "/*<*/";
   if (w->styleSheet().contains(token)) return;
   w->setStyleSheet(w->styleSheet().append(token));
}

static void styleSheetClear(QWidget * w, const QString & what)
{
   if (what.isEmpty()) return;
   QString token = "/*>*/" + what + "/*<*/";
   if (! w->styleSheet().contains(token)) return;
   w->setStyleSheet(w->styleSheet().remove(token));
}

static void styleSheetSelect(QWidget * w, bool selector,
                             const QString & onTrue,
                             const QString & onFalse = QString())
{
   styleSheetSet(w, selector ? onTrue : onFalse);
   styleSheetClear(w, selector ? onFalse : onTrue);
}

template <typename T, typename U>
static void setSelect(QSet<T> & set, bool b, const U & val) {
   if (b) set.insert(val); else set.remove(val);
}

class DialogValidator : public QObject {
   Q_OBJECT
   QSet<QWidget*> m_validWidgets;
   int m_needsValid;
   Q_SLOT void checkValidity() {
      checkValidity(qobject_cast<QLineEdit*>(sender()));
   }
   void checkValidity(QLineEdit * l) {
      if (!l) return;
      QString text = l->text();
      int pos;
      bool isValid =
            l->validator()->validate(text, pos) == QValidator::Acceptable;
      setSelect(m_validWidgets, isValid, (QWidget*)l);
      styleSheetSelect(l, !isValid,
                       QStringLiteral("QLineEdit { background: yellow }"));
      emit newValidity(m_validWidgets.count() == m_needsValid);
   }
public:
   DialogValidator(QObject * parent = 0) : QObject(parent), m_needsValid(0) {}
   void add(QLineEdit * l, QValidator * v) {
      l->setValidator(v);
      connect(l, SIGNAL(textChanged(QString)), SLOT(checkValidity()));
      m_needsValid ++;
      checkValidity(l);
   }
   Q_SIGNAL void newValidity(bool);
};

class MyDialog : public QDialog {
   QFormLayout m_layout;
   QLineEdit m_cFactor;
   QLineEdit m_dFactor;
   QDialogButtonBox m_buttons;
   DialogValidator m_validator;
public:
   MyDialog(QWidget * parent = 0, Qt::WindowFlags f = 0) :
      QDialog(parent, f), m_layout(this),
      m_buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
   {
      m_layout.addRow("Combobulation Factor", &m_cFactor);
      m_layout.addRow("Decombobulation Factor", &m_dFactor);
      m_layout.addRow(&m_buttons);
      connect(&m_buttons, SIGNAL(accepted()), SLOT(accept()));
      connect(&m_buttons, SIGNAL(rejected()), SLOT(reject()));
      connect(&m_validator, SIGNAL(newValidity(bool)),
              m_buttons.button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
      m_validator.add(&m_cFactor, new QIntValidator(0, 50, this));
      m_validator.add(&m_dFactor, new QIntValidator(-50, 0, this));
   }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   MyDialog d;
   d.show();
   return a.exec();
}

#include "main.moc"
