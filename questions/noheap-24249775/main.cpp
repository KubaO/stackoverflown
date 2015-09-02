// Interface
#include <QDialog>
#include <QGridLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QDialogButtonBox>

#if QT_VERSION<QT_VERSION_CHECK(5,0,0)
#define Q_DECL_OVERRIDE
#endif

class FindDialog: public QDialog {
   struct Ui {
      QGridLayout layout;
      QCheckBox regex, caseSense, select, whole, forward, wrap;
      QLineEdit exprBox;
      QDialogButtonBox buttonBox;
      Ui(QWidget * widget);
   } m_ui;
   void get();
public:
   bool regex, caseSensitive, inSelection, wholeWord, forward, wrap;
   QString expr;

   FindDialog(QWidget *parent = 0);
   ~FindDialog();
   void set();
   void done(int r) Q_DECL_OVERRIDE;
};

// Implementation
FindDialog::Ui::Ui(QWidget * widget) :
   layout(widget),
   regex(tr("Regex")),
   caseSense(tr("Match case")),
   select(tr("In Selection")),
   whole(tr("Whole Word")),
   forward(tr("Forward")),
   wrap(tr("Wrap around")),
   buttonBox(QDialogButtonBox::Cancel)
{
   layout.addWidget(&regex, 0, 0);
   layout.addWidget(&caseSense, 0, 1);
   layout.addWidget(&select, 0, 2);
   layout.addWidget(&whole, 1, 0);
   layout.addWidget(&forward, 1, 1);
   layout.addWidget(&wrap, 1, 2);
   layout.addWidget(&exprBox, 2, 0, 1, 3);
   layout.addWidget(&buttonBox, 3, 0, 1, 3);
   exprBox.setPlaceholderText(tr("Find expr..."));
   buttonBox.addButton(tr("Find"), QDialogButtonBox::AcceptRole);
}

FindDialog::FindDialog(QWidget *parent): QDialog(parent), m_ui(this),
   regex(false), caseSensitive(true), inSelection(false), wholeWord(true),
   forward(true), wrap(true)
{
   set();
   connect(&m_ui.buttonBox, SIGNAL(rejected()), SLOT(reject()));
   connect(&m_ui.buttonBox, SIGNAL(accepted()), SLOT(accept()));
   setWindowTitle("Find");
}

FindDialog::~FindDialog() {}

void FindDialog::done(int result)
{
   get();
   QDialog::done(result);
}

void FindDialog::get()
{
   regex = m_ui.regex.isChecked();
   caseSensitive = m_ui.caseSense.isChecked();
   inSelection = m_ui.select.isChecked();
   wholeWord = m_ui.whole.isChecked();
   forward = m_ui.forward.isChecked();
   wrap = m_ui.wrap.isChecked();
   expr = m_ui.exprBox.text();
}

void FindDialog::set()
{
   m_ui.regex.setChecked(regex);
   m_ui.caseSense.setChecked(caseSensitive);
   m_ui.select.setChecked(inSelection);
   m_ui.whole.setChecked(wholeWord);
   m_ui.forward.setChecked(forward);
   m_ui.wrap.setChecked(wrap);
}

// Main
#include <QApplication>
#include <QMessageBox>

int main(int argc, char **argv) {
   QApplication app(argc, argv);
   app.setApplicationName("NIDE");
   FindDialog fd;
   fd.show();
#if QT_VERSION>=QT_VERSION_CHECK(5,0,0)
   // This can't be done in Qt4 without using moc
   QObject::connect(&fd, &QDialog::accepted, [&fd]{
      QMessageBox::information(NULL, "Find",
         QString("The user wants to find \"%1\"").arg(fd.expr));
   });
#endif
   return app.exec();
}
