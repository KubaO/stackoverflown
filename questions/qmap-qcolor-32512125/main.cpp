// https://github.com/KubaO/stackoverflown/tree/master/questions/qmap-qcolor-32512125
#include <QtGui>

bool operator<(const QColor & a, const QColor & b) {
   return a.redF() < b.redF()
       || a.greenF() < b.greenF()
       || a.blueF() < b.blueF()
       || a.alphaF() < b.alphaF();
}

int main() {
   Q_ASSERT(QColor(Qt::blue) < QColor(Qt::red));
   Q_ASSERT(QColor(Qt::green) < QColor(Qt::red));
   Q_ASSERT(QColor(Qt::blue) < QColor(Qt::green));
   Q_ASSERT(! (QColor(Qt::red) < QColor(Qt::red)));
   QMap<QColor, int> map;
   map.insert(Qt::red, 0);
   map.insert(Qt::green, 1);
   map.insert(Qt::blue, 2);
   Q_ASSERT(map.size() == 3);
   Q_ASSERT(map.cbegin().key() == Qt::red);
   Q_ASSERT((map.cbegin()+1).key() == Qt::green);
   Q_ASSERT((map.cbegin()+2).key() == Qt::blue);
   qDebug() << map;
}

