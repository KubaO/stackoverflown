// https://github.com/KubaO/stackoverflown/tree/master/questions/buttonbox-22404318
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif
#include <type_traits>

void styleSheetSet(QWidget *w, const QString &what) {
   auto const token = QStringLiteral("/*>*/%1/*<*/").arg(what);
   if (what.isEmpty() || w->styleSheet().contains(token)) return;
   w->setStyleSheet(w->styleSheet().append(token));
}

void styleSheetClear(QWidget *w, const QString &what) {
   const auto token = QStringLiteral("/*>*/%1/*<*/").arg(what);
   if (what.isEmpty() || ! w->styleSheet().contains(token)) return;
   w->setStyleSheet(w->styleSheet().remove(token));
}

void styleSheetSelect(QWidget *w, bool selector,
                      const QString & onTrue,
                      const QString & onFalse = {})
{
   styleSheetSet(w, selector ? onTrue : onFalse);
   styleSheetClear(w, selector ? onFalse : onTrue);
}

template <typename T, typename U>
void setSelect(QSet<T> &set, bool b, const U &val) {
   if (b) set.insert(val); else set.remove(val);
}

bool hasParent(QObject *obj, QObject *const parent) {
   Q_ASSERT(obj);
   while ((obj = obj->parent()))
      if (obj == parent) return true;
   return obj == parent;
}

class DialogValidator : public QObject {
   Q_OBJECT
   QSet<QWidget*> m_validWidgets;
   int m_needsValid = 0;
   Q_SLOT void checkWidget() {
      if (sender()->isWidgetType())
         checkValidity(static_cast<QWidget*>(sender()));
   }
   Q_SLOT void checkLineEdit() {
      if (auto *l = qobject_cast<QLineEdit*>(sender()))
         checkValidity(l);
   }
   void checkValidity(QLineEdit *l) {
      indicateValidity(l, l->hasAcceptableInput());
   }
   void checkValidity(QWidget *w) {
      auto validator = w->findChild<QValidator*>();
      if (!validator) return;
      auto prop = w->metaObject()->userProperty();
      QVariant value = prop.read(w);
      int pos;
      QString text = value.toString();
      bool isValid =
            validator->validate(text, pos) == QValidator::Acceptable;
      indicateValidity(w, isValid);
   }
   void indicateValidity(QWidget *w, bool isValid) {
      auto *combo = qobject_cast<QComboBox*>(w->parentWidget());
      setSelect(m_validWidgets, isValid, combo ? combo : w);
      styleSheetSelect(w, !isValid,
                       QStringLiteral("%1 { background: yellow }")
                       .arg(QLatin1String(w->metaObject()->className())));
      emit newValidity(m_validWidgets.count() == m_needsValid);
   }
   template<typename W>
   typename std::enable_if<!std::is_same<QWidget,W>::value, bool>::type
   addPoly(W* w, QValidator *v) {
      if (!w) return false;
      return (add(w,v), true);
   }
public:
   DialogValidator(QObject *parent = {}) : QObject(parent) {}
   Q_SIGNAL void newValidity(bool);
   void addPoly(QWidget *w, QValidator *v) {
      addPoly(qobject_cast<QLineEdit*>(w), v) ||
            addPoly(qobject_cast<QComboBox*>(w), v) ||
            (static_cast<void>(add(w, v)), true);
   }
   void add(QComboBox *b, QValidator *v) {
      if (auto *l = b->lineEdit())
         add(l, v);
   }
   void add(QLineEdit *l, QValidator *v) {
      l->setValidator(v);
      connect(l, SIGNAL(textChanged(QString)), SLOT(checkLineEdit()));
      m_needsValid++;
      checkValidity(l);
   }
   void add(QWidget *w, QValidator *v) {
      Q_ASSERT(hasParent(v, w));
      auto prop = w->metaObject()->userProperty();
      auto propChanged = prop.notifySignal();
      static auto check = metaObject()->method(metaObject()->indexOfSlot("checkWidget()"));
      Q_ASSERT(check.isValid());
      if (!prop.isValid() || !propChanged.isValid())
         return qWarning("DialogValidator::add: The target widget has no user property with a notifier.");
      if (connect(w, propChanged, this, check)) {
         m_needsValid++;
         checkValidity(w);
      }
   }
   template <typename V, typename W, typename...Args>
   typename std::enable_if<
   std::is_base_of<QWidget, W>::value && std::is_base_of<QValidator, V>::value, V*>::type
   add(W *w, Args...args) {
      V *validator = new V(std::forward<Args>(args)..., w);
      return add(w, validator), validator;
   }
};

class MyDialog : public QDialog {
   Q_OBJECT
   QFormLayout m_layout{this};
   QLineEdit m_cFactor;
   QLineEdit m_dFactor;
   QDialogButtonBox m_buttons{QDialogButtonBox::Ok | QDialogButtonBox::Cancel};
   DialogValidator m_validator;
public:
   MyDialog(QWidget *parent = {}, Qt::WindowFlags f = {}) : QDialog(parent, f) {
      m_layout.addRow("Combobulation Factor", &m_cFactor);
      m_layout.addRow("Decombobulation Factor", &m_dFactor);
      m_layout.addRow(&m_buttons);
      connect(&m_buttons, SIGNAL(accepted()), SLOT(accept()));
      connect(&m_buttons, SIGNAL(rejected()), SLOT(reject()));
      connect(&m_validator, SIGNAL(newValidity(bool)),
              m_buttons.button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
      // QLineEdit-specific validator
      m_validator.add<QIntValidator>(&m_cFactor, 0, 50);
      // Generic user property-based validator
      m_validator.add<QIntValidator, QWidget>(&m_dFactor, -50, 0);
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
