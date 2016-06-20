// https://github.com/KubaO/stackoverflown/tree/master/questions/simple-parse-37902620
#include <QtCore>

void callMethod(const QString & a, const QString & b, int c) {
   qDebug() << "read" << a << b << c;
}

void parse(QIODevice * input)
{
   QTextStream in(input);
   while (!in.atEnd()) {
      auto const line = in.readLine();
      auto const parts = line.split(':');
      if (parts.size() == 3) {
         bool ok;
         auto p3 = parts.at(2).toInt(&ok);
         if (ok)
            callMethod(parts.at(0).trimmed(), parts.at(1).trimmed(), p3);
      }
   }
}

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   QByteArray data{" a1 :  b1  : 10   \nLol:cats: 33"};
   QBuffer buf(&data);
   if (buf.open(QIODevice::ReadOnly))
      parse(&buf);
}
