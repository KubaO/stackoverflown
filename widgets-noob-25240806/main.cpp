#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QGridLayout>
#include <QDoubleSpinBox>
#include <QVector3D>

class MainWindow : public QMainWindow {
   QWidget m_central;
   QGridLayout m_centralLayout;
   QDoubleSpinBox m_oldX, m_oldY, m_oldZ;
   QLabel m_lblX, m_lblY, m_lblZ;
public:
   MainWindow(QWidget * parent = 0, Qt::WindowFlags flags = 0);
};

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags) :
   QMainWindow(parent, flags),
   m_centralLayout(&m_central),
   m_lblX("Old X Coord: "),
   m_lblY("Old Y Coord: "),
   m_lblZ("Old Z Coord: ")
{
   QList<QDoubleSpinBox*> spins;
   spins << &m_oldX << &m_oldY << &m_oldZ;
   int i = 0;
   foreach (QDoubleSpinBox * spin, spins) {
      spin->setRange(0.0, 1.0);
      spin->setDecimals(3);
      spin->setSuffix(" km");
      m_centralLayout.addWidget(spin, i++, 1);
   }

   m_centralLayout.addWidget(&m_lblX, 0, 0);
   m_centralLayout.addWidget(&m_lblY, 1, 0);
   m_centralLayout.addWidget(&m_lblZ, 2, 0);

   setCentralWidget(&m_central);
}

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   MainWindow w;
   w.show();
   return a.exec();
}
