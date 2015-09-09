// https://github.com/KubaO/stackoverflown/tree/master/tumbleweeds/datastream-ops-32447800
#include <QtCore>

int main() {
   auto string = QStringLiteral("FooBar");
   QByteArray block1, block2;
   QDataStream out1(&block1, QIODevice::WriteOnly), out2(&block2, QIODevice::WriteOnly);
   out1 << quint16(0) << string;
   operator<<(out2.operator<<(quint16(0)), string);
   Q_ASSERT(block1 == block2);
}

