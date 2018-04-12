// https://github.com/KubaO/stackoverflown/tree/master/questions/stream-49779857
#include <QtCore>

QByteArray readAll(const QString &fileName) {
   QFile f(fileName);
   if (f.open(QIODevice::ReadOnly))
      return f.readAll();
   return {};
}

int main() {
   auto tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
   auto fileName = QStringLiteral("%1/com.stackoverflow.questions.49779857-result.txt")
         .arg(tmp);
   QFile file(fileName);
   file.remove();
   if (!file.open(QIODevice::Append))
      qFatal("Cannot open file!");

   QTextStream ts(&file);
   auto num = 1.0f, error = 2.0f;
   ts << "num=" << num << "\t" << "error=" << error << endl;
   file.close();

   Q_ASSERT(file.exists());
   Q_ASSERT(readAll(fileName) == "num=1\terror=2\n");
}

