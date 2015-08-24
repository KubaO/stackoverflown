#include <QtWidgets>

namespace UDial_ {
class CUDial : public QWidget {
   Q_OBJECT
public:
   CUDial(QWidget *parent = 0) : QWidget(parent) {}
};

class DUDial : public QWidget
{
   Q_OBJECT
public:
   DUDial(QWidget *parent = 0) : QWidget(parent) {}
};
}

class UDial : public QDialog
{
   Q_OBJECT
   QVBoxLayout vlay { this };
   QTabWidget tab;
   // We make it explicit that both sub-dials are children of tab and must not
   // be declared / initialized before it!
   UDial_::CUDial wid1 { &tab };
   UDial_::DUDial wid2 { &tab };
   QDialogButtonBox box;

public:
   UDial(QWidget *parent = 0);
};

UDial::UDial(QWidget * parent) :
   QDialog(parent),
   box(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
{
   vlay.addWidget(&tab);
   vlay.addWidget(&box);
   tab.addTab(&wid1, "Dial 1");
   tab.addTab(&wid2, "Dial 2");
}

int main(int argc, char ** argv)
{
   QApplication app(argc, argv);
   UDial dial;
   dial.show();
   return app.exec();
}

#include "main.moc"
