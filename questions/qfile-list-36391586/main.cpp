// https://github.com/KubaO/stackoverflown/tree/master/questions/qfile-list-36391586
#include <QtCore>
#include <list>

void populate(std::list<QFile> & files) {
   QDir dir(QCoreApplication::applicationDirPath());
   auto entries = dir.entryList(QDir::Files);
   for (auto fileName : dir.entryList(QDir::Files)) {
      qDebug() << "adding file" << fileName;
      files.emplace_back(fileName);
   }
}

void iterate(std::list<QFile> & files) {
   for (auto & file : files)
      if (file.open(QIODevice::ReadOnly)) {
         qDebug() << "successfully opened" << file.fileName();
      }
}

int main(int argc, char ** argv) {
   QCoreApplication app(argc, argv);
   std::list<QFile> files;
   populate(files);
   iterate(files);
}

