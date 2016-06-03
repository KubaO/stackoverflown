// https://github.com/KubaO/stackoverflown/tree/master/questions/dupechecker-37557870
#include <QtCore>
#include <cstdio>
QTextStream out(stdout);
QTextStream err(stderr);

int check(const QString & path) {
   int unique = 0;
   //   size         hash        path
   QMap<qint64, QMap<QByteArray, QString>> entries;
   QDirIterator it(path, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
   QCryptographicHash hash{QCryptographicHash::Sha256};
   while (it.hasNext()) {
      it.next();
      auto const info = it.fileInfo();
      if (info.isDir()) continue;
      auto const size = info.size();
      if (size == 0) continue; // all zero-sized files are "duplicates" but let's ignore them
      auto const path = info.absoluteFilePath();

      QFile file(path); // RAII class, no need to explicitly close
      if (!file.open(QIODevice::ReadOnly)) {
         err << "Can't open " << path << endl;
         continue;
      }
      hash.reset();
      hash.addData(&file);
      if (file.error() != QFile::NoError) {
         err << "Error reading " << path << endl;
         continue;
      }
      auto & dupe = entries[size][hash.result()];
      if (! dupe.isNull()) {
         // duplicate
         out << path << " is a duplicate of " << dupe << endl;
      } else {
         dupe = path;
         ++ unique;
      }
   }
   return unique;
}

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   QDir dir;
   if (argc == 2)
      dir = app.arguments().at(1);
   auto unique = check(dir.absolutePath());
   out << "Finished. There were " << unique << " unique files." << endl;
}
