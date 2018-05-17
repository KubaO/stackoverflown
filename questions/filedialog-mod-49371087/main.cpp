// filedialog-mod-49371087
#include <QtWidgets>

class FileDialogModder : QObject {
public:
   enum Option { SpaceButtons = 1 };
   Q_DECLARE_FLAGS(Options, Option)
   FileDialogModder(QFileDialog *dialog, Options options = {});
   void relayout(int extraRows);
   void addButton(QDialogButtonBox::StandardButton);
   QGridLayout *layout() const { return m_layout; }
   QDialogButtonBox *buttonBox() const { return m_buttonBox; }
   int row() const { return 3; }
   int colSpan() const { return 3; }
private:
   Options m_options;
   QDialog *m_dialog;
   QGridLayout *m_layout;
   QDialogButtonBox *m_buttonBox;
   QWidget *m_fileNameLabel, *m_fileNameEdit, *m_fileTypeLabel, *m_fileTypeCombo;
   bool isAt(QWidget *, int row, int col, int rowSpan, int ColSpan) const;
   QList<QAbstractButton*> buttons() const {
      return findChildren<QAbstractButton*>(kModButton); }
   static char kModButton[];
};
Q_DECLARE_OPERATORS_FOR_FLAGS(FileDialogModder::Options)
char FileDialogModder::kModButton[] = "modButton";

FileDialogModder::FileDialogModder(QFileDialog *dialog, FileDialogModder::Options options) :
   QObject((dialog->setOption(QFileDialog::DontUseNativeDialog), dialog)),
   m_options{options},
   m_dialog{dialog},
   m_layout{qobject_cast<QGridLayout*>(dialog->layout())},
   m_buttonBox{dialog->findChild<QDialogButtonBox*>("buttonBox")},
   m_fileNameLabel{dialog->findChild<QLabel*>("fileNameLabel")},
   m_fileNameEdit{dialog->findChild<QLineEdit*>("fileNameEdit")},
   m_fileTypeLabel{dialog->findChild<QLabel*>("fileTypeLabel")},
   m_fileTypeCombo{dialog->findChild<QComboBox*>("fileTypeCombo")}
{
   for (auto *b : m_buttonBox->findChildren<QAbstractButton*>())
      b->setObjectName(kModButton);
   Q_ASSERT((isAt(m_buttonBox, 2, 2, 2, 1)));
   Q_ASSERT((isAt(m_fileNameLabel, 2, 0, 1, 1)));
   Q_ASSERT((isAt(m_fileNameEdit, 2, 1, 1, 1)));
   Q_ASSERT((isAt(m_fileTypeLabel, 3, 0, 1, 1)));
   Q_ASSERT((isAt(m_fileTypeCombo, 3, 1, 1, 1)));
   Q_ASSERT(buttons().size() == 2);
}

bool FileDialogModder::isAt(QWidget *w, int row, int col, int rowSpan, int colSpan) const {
   int index = m_layout->indexOf(w);
   if (index < -1)
      return false;
   int r, c, rs, cs;
   m_layout->getItemPosition(index, &r, &c, &rs, &cs);
   return r==row && c==col && rs==rowSpan && cs==colSpan;
}

void FileDialogModder::relayout(int extraRows) {
   // remove widgets and layouts
   for (auto *w : {(QWidget*)m_buttonBox, m_fileTypeLabel, m_fileTypeCombo})
      m_layout->removeWidget(w);

   // add widgets in new locations
   if (m_options & SpaceButtons) {
      auto const buttons = this->buttons();
      for (auto *b : buttons) b->setParent(m_dialog);
      m_buttonBox->hide();
      if (!buttons.isEmpty()) {
         m_layout->addWidget(buttons[0], 2, 2, 1, 1);
         auto it = std::prev(buttons.end());
         for (int r = 2+extraRows; r > 2 && it != buttons.begin(); --r, --it)
            m_layout->addWidget(*it, r, 2, 1, 1);
      }
   } else {
      m_layout->addWidget(m_buttonBox, 2, 2, 3, 1);
   }
   m_layout->addWidget(m_fileTypeLabel, 4, 0, 1, 1);
   m_layout->addWidget(m_fileTypeCombo, 4, 1, 1, 1);
}

void FileDialogModder::addButton(QDialogButtonBox::StandardButton button) {
   if (!m_options & SpaceButtons)
   m_buttonBox->setStandardButtons(m_buttonBox->standardButtons() | button);


}

int main(int argc, char **argv) {
   QApplication app{argc, argv};
   QFileDialog dialog;
   FileDialogModder mod(&dialog, FileDialogModder::SpaceButtons);
   mod.relayout(2);
   // add an extra button and some space
   mod.addButton(QDialogButtonBox::Ignore);
   mod.layout()->addItem(new QSpacerItem(0, 50), mod.row(), 0);
   dialog.show();
   return app.exec();
}
