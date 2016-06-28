// https://github.com/KubaO/stackoverflown/tree/master/questions/find-hide-38082794
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

class DocumentationWin : public QDialog
{
   Q_OBJECT
public:
   explicit DocumentationWin(QWidget * parent = 0);
private:
   QVBoxLayout layout{this};
   QToolBar toolbar;
   QTextBrowser docs;
   QWidget bottom;
   QHBoxLayout footer{&bottom};
   QLabel searchlabel{tr("Find in page:")};
   QLabel resultslabel;
   QLineEdit searchinput;
   QToolButton findprev;
   QToolButton findnext;
   QCheckBox casecheckbox{tr("Case sensitive")};
   QPushButton closeButton;

   Q_SLOT void onFindPrev() {}
   Q_SLOT void onFindNext() {}
};

DocumentationWin::DocumentationWin(QWidget * parent) : QDialog(parent) {
   findprev.setArrowType(Qt::UpArrow);
   connect(&findprev, SIGNAL(clicked()), this, SLOT(onFindPrev()));
   findnext.setArrowType(Qt::DownArrow);
   connect(&findnext, SIGNAL(clicked()), this, SLOT(onFindNext()));

   auto style = qApp->style();
   auto closeIcon = style->standardIcon(QStyle::SP_TitleBarCloseButton);
   closeButton.setIcon(closeIcon);
   closeButton.setFlat(true);
   connect(&closeButton, SIGNAL(clicked(bool)), &bottom, SLOT(hide()));

   footer.setContentsMargins(5,5,5,5);
   footer.addWidget(&searchlabel);
   footer.addSpacing(3);
   footer.addWidget(&searchinput);
   footer.addWidget(&findprev);
   footer.addWidget(&findnext);
   footer.addSpacing(10);
   footer.addWidget(&casecheckbox);
   footer.addSpacing(10);
   footer.addWidget(&resultslabel);
   footer.addStretch(1);
   footer.addWidget(&closeButton);

   layout.setContentsMargins(0,0,0,0);
   layout.setSpacing(0);
   layout.addWidget(&toolbar);
   layout.addWidget(&docs);
   layout.addWidget(&bottom);
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   DocumentationWin win;
   win.show();
   return app.exec();
}

#include "main.moc"
