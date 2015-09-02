#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

QPair<int,int> gridPosition(QWidget * widget) {
  auto gp = qMakePair(-1,-1);
  if (! widget->parentWidget()) return gp;
  auto layout = dynamic_cast<QGridLayout*>(widget->parentWidget()->layout());
  if (! layout) return gp;
  int index = layout->indexOf(widget);
  Q_ASSERT(index >= 0);
  int rs,cs;
  layout->getItemPosition(index, &gp.first, &gp.second, &rs, &cs);
  return gp;
}

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QWidget w;
   QGridLayout l(&w);
   QLabel gridPos;
   l.addWidget(&gridPos, 0, 0, 1, 4);
   for (int i = 1; i < 4; ++ i)
      for (int j = 0; j < 3; ++ j) {
         auto b = new QPushButton(QString("%1,%2").arg(i).arg(j));
         l.addWidget(b, i, j);
         QObject::connect(b, &QPushButton::clicked, [&gridPos, b]{
            auto p = gridPosition(b);
            gridPos.setText(QString("Grid Pos: %1,%2")
                            .arg(p.first).arg(p.second));
         });
      }
   w.show();
   return a.exec();
}
