#if 1
#include <QApplication>
#include <QDialog>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>

class TabDialog : public QDialog
{
   QVBoxLayout m_layout;
   QTabWidget m_tabWidget;
   QDialogButtonBox m_buttonBox;
   QWidget m_generalTab, m_permissionsTab, m_applicationsTab;
public:
   TabDialog() :
      m_layout(this),
      m_buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
   {
      m_tabWidget.addTab(&m_generalTab, tr("General"));
      m_tabWidget.addTab(&m_permissionsTab, tr("Permissions"));
      m_tabWidget.addTab(&m_applicationsTab, tr("Applications"));
      m_layout.addWidget(&m_tabWidget);
      m_layout.addWidget(&m_buttonBox);
      connect(&m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
      connect(&m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
   }
};

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   auto tabDialog = new TabDialog();
   tabDialog->setAttribute(Qt::WA_DeleteOnClose);
   tabDialog->show();
   return app.exec();
}
#endif

#if 0
#include <QApplication>
#include <QDialog>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>

class TabDialog : public QDialog
{
   QTabWidget *tabWidget;
   QDialogButtonBox *buttonBox;
public:
   TabDialog() :
      tabWidget(new QTabWidget),
      buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok |
                                     QDialogButtonBox::Cancel))
   {
      tabWidget->addTab(new QWidget, tr("General"));
      tabWidget->addTab(new QWidget, tr("Permissions"));
      tabWidget->addTab(new QWidget, tr("Applications"));
      QVBoxLayout *layout = new QVBoxLayout;
      layout->addWidget(tabWidget);
      layout->addWidget(buttonBox);
      setLayout(layout);
      connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
      connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
   }
   ~TabDialog() {
      Q_ASSERT(findChild<QTabWidget*>());
      Q_ASSERT(findChild<QDialogButtonBox*>());
      delete tabWidget;
      Q_ASSERT(! findChild<QTabWidget*>());
      Q_ASSERT(findChild<QDialogButtonBox*>());
      delete buttonBox;
      Q_ASSERT(! findChild<QTabWidget*>());
      Q_ASSERT(! findChild<QDialogButtonBox*>());
   }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   TabDialog *tabDialog = new TabDialog();
   tabDialog->setAttribute(Qt::WA_DeleteOnClose);
   tabDialog->exec();
   return 0;
}
#endif

#if 0
// This is Qt example code, modified per the question just to check if the question includes
// code that actually fails.
#include <QtWidgets>

class GeneralTab : public QWidget
{
   Q_OBJECT
public:
   explicit GeneralTab(const QFileInfo &fileInfo, QWidget *parent = 0) : QWidget(parent)
   {
      QLabel *fileNameLabel = new QLabel(tr("File Name:"));
      QLineEdit *fileNameEdit = new QLineEdit(fileInfo.fileName());

      QLabel *pathLabel = new QLabel(tr("Path:"));
      QLabel *pathValueLabel = new QLabel(fileInfo.absoluteFilePath());
      pathValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

      QLabel *sizeLabel = new QLabel(tr("Size:"));
      qlonglong size = fileInfo.size()/1024;
      QLabel *sizeValueLabel = new QLabel(tr("%1 K").arg(size));
      sizeValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

      QLabel *lastReadLabel = new QLabel(tr("Last Read:"));
      QLabel *lastReadValueLabel = new QLabel(fileInfo.lastRead().toString());
      lastReadValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

      QLabel *lastModLabel = new QLabel(tr("Last Modified:"));
      QLabel *lastModValueLabel = new QLabel(fileInfo.lastModified().toString());
      lastModValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

      QVBoxLayout *mainLayout = new QVBoxLayout;
      mainLayout->addWidget(fileNameLabel);
      mainLayout->addWidget(fileNameEdit);
      mainLayout->addWidget(pathLabel);
      mainLayout->addWidget(pathValueLabel);
      mainLayout->addWidget(sizeLabel);
      mainLayout->addWidget(sizeValueLabel);
      mainLayout->addWidget(lastReadLabel);
      mainLayout->addWidget(lastReadValueLabel);
      mainLayout->addWidget(lastModLabel);
      mainLayout->addWidget(lastModValueLabel);
      mainLayout->addStretch(1);
      setLayout(mainLayout);
   }
};

class PermissionsTab : public QWidget
{
   Q_OBJECT
public:
   explicit PermissionsTab(const QFileInfo &fileInfo, QWidget *parent = 0): QWidget(parent)
   {
      QGroupBox *permissionsGroup = new QGroupBox(tr("Permissions"));

      QCheckBox *readable = new QCheckBox(tr("Readable"));
      if (fileInfo.isReadable())
         readable->setChecked(true);

      QCheckBox *writable = new QCheckBox(tr("Writable"));
      if ( fileInfo.isWritable() )
         writable->setChecked(true);

      QCheckBox *executable = new QCheckBox(tr("Executable"));
      if ( fileInfo.isExecutable() )
         executable->setChecked(true);

      QGroupBox *ownerGroup = new QGroupBox(tr("Ownership"));

      QLabel *ownerLabel = new QLabel(tr("Owner"));
      QLabel *ownerValueLabel = new QLabel(fileInfo.owner());
      ownerValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

      QLabel *groupLabel = new QLabel(tr("Group"));
      QLabel *groupValueLabel = new QLabel(fileInfo.group());
      groupValueLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

      QVBoxLayout *permissionsLayout = new QVBoxLayout;
      permissionsLayout->addWidget(readable);
      permissionsLayout->addWidget(writable);
      permissionsLayout->addWidget(executable);
      permissionsGroup->setLayout(permissionsLayout);

      QVBoxLayout *ownerLayout = new QVBoxLayout;
      ownerLayout->addWidget(ownerLabel);
      ownerLayout->addWidget(ownerValueLabel);
      ownerLayout->addWidget(groupLabel);
      ownerLayout->addWidget(groupValueLabel);
      ownerGroup->setLayout(ownerLayout);

      QVBoxLayout *mainLayout = new QVBoxLayout;
      mainLayout->addWidget(permissionsGroup);
      mainLayout->addWidget(ownerGroup);
      mainLayout->addStretch(1);
      setLayout(mainLayout);
   }
};

class ApplicationsTab : public QWidget
{
   Q_OBJECT
public:
   explicit ApplicationsTab(const QFileInfo &fileInfo, QWidget *parent = 0) : QWidget(parent)
   {
      QLabel *topLabel = new QLabel(tr("Open with:"));

      QListWidget *applicationsListBox = new QListWidget;
      QStringList applications;

      for (int i = 1; i <= 30; ++i)
         applications.append(tr("Application %1").arg(i));
      applicationsListBox->insertItems(0, applications);

      QCheckBox *alwaysCheckBox;

      if (fileInfo.suffix().isEmpty())
         alwaysCheckBox = new QCheckBox(tr("Always use this application to "
                                           "open this type of file"));
      else
         alwaysCheckBox = new QCheckBox(tr("Always use this application to "
                                           "open files with the extension '%1'").arg(fileInfo.suffix()));

      QVBoxLayout *layout = new QVBoxLayout;
      layout->addWidget(topLabel);
      layout->addWidget(applicationsListBox);
      layout->addWidget(alwaysCheckBox);
      setLayout(layout);
   }
};

class TabDialog : public QDialog
{
   Q_OBJECT
   QTabWidget *tabWidget;
   QDialogButtonBox *buttonBox;
public:
   explicit TabDialog(const QString &fileName, QWidget *parent = 0): QDialog(parent)
   {
      QFileInfo fileInfo(fileName);

      tabWidget = new QTabWidget;
      tabWidget->addTab(new GeneralTab(fileInfo), tr("General"));
      tabWidget->addTab(new PermissionsTab(fileInfo), tr("Permissions"));
      tabWidget->addTab(new ApplicationsTab(fileInfo), tr("Applications"));

      buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                       | QDialogButtonBox::Cancel);

      connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
      connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

      QVBoxLayout *mainLayout = new QVBoxLayout;
      mainLayout->addWidget(tabWidget);
      mainLayout->addWidget(buttonBox);
      setLayout(mainLayout);

      setWindowTitle(tr("Tab Dialog"));
   }
   ~TabDialog() {
      delete tabWidget;
      delete buttonBox;
   }
};

int main(int argc, char ** argv)
{
   QApplication app(argc, argv);
   QString fileName = argc >= 2 ? argv[1] : ".";
   TabDialog *tabDialog = new TabDialog(fileName);
   tabDialog->setAttribute(Qt::WA_DeleteOnClose);
   tabDialog->exec();
   return 0;
}

#include "main.moc"

#endif
