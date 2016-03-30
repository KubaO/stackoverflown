// https://github.com/KubaO/stackoverflown/tree/master/questions/str-split-36297178
#include <QtCore>

QVector<QStringList> splitTerms(const QStringList & source)
{
   QVector<QStringList> result;
   result.reserve(source.count());
   for (auto src : source)
      result.append(src.split(QChar('*'), QString::SkipEmptyParts));
   return result;
}

int main() {
   qDebug() << splitTerms(QStringList{"k*k1*k2", "s*s1*s2", "b*b1*b2"});
}
