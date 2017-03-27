// https://github.com/KubaO/stackoverflown/tree/master/questions/resource-bypass-43044268
#include <QtCore>

const char kInsertsFile[] = ":/insertstatements.txt";

QString toWritableName(const QString & qrcFileName) {
   Q_ASSERT (qrcFileName.startsWith(":/"));
   QFileInfo info(qrcFileName);
   return
         QStandardPaths::writableLocation(QStandardPaths::DataLocation)
         + info.path().mid(1) + '/' + info.fileName();
}

QString toReadableName(const QString & qrcFileName) {
   Q_ASSERT (qrcFileName.startsWith(":/"));
   auto writable = toWritableName(qrcFileName);
   return QFileInfo(writable).exists() ? writable : qrcFileName;
}

bool setupWritableFile(QSaveFile & dst, QIODevice::OpenMode mode = {}) {
   Q_ASSERT (dst.fileName().startsWith(":/"));
   Q_ASSERT (mode == QIODevice::OpenMode{} || mode == QIODevice::Text);
   QFile src(toReadableName(dst.fileName()));
   dst.setFileName(toWritableName(dst.fileName()));
   if (!src.open(QIODevice::ReadOnly | mode))
      return false;
   auto data = src.readAll();
   src.close(); // Don't keep the file descriptor tied up any longer.
   QFileInfo dstInfo(dst.fileName());
   if (!dstInfo.dir().exists() && !QDir().mkpath(dstInfo.path()))
      return false;
   if (!dst.open(QIODevice::WriteOnly | mode))
      return false;
   return dst.write(data) == data.size();
}

bool addInsertToFile(const QString & insert) {
   QSaveFile file(kInsertsFile);
   if (!setupWritableFile(file, QIODevice::Text))
      return false;
   if (true) {
      // Alternative 1
      QTextStream s(&file);
      s << insert << '\n';
   } else {
      // Alternative 2
      file.write((insert + '\n').toLocal8Bit());
   }
   return file.commit();
}

QStringList readInserts() {
   QFile file(toReadableName(kInsertsFile));
   if (!file.open(QIODevice::ReadOnly))
      return {};
   return QString::fromLocal8Bit(file.readAll()).split('\n', QString::SkipEmptyParts);
}

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   app.setApplicationName("resource-bypass-42044268");
   qDebug() << "Original Inserts:" << readInserts();
   auto rc = addInsertToFile("NewInsert");
   qDebug() << "Modification status:" << rc;
   qDebug() << "Current Inserts:" << readInserts();
}
