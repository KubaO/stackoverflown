// https://github.com/KubaO/stackoverflown/tree/master/questions/filemodel-18548048
#include <QtWidgets>
#include <QtConcurrent>

void makeLines(QBuffer &buf, int count = 1000000) {
   buf.open(QIODevice::WriteOnly | QIODevice::Text);
   char line[16];
   for (int i = 0; i < count; ++i) {
      int n = qsnprintf(line, sizeof(line), "Item %d\n", i);
      buf.write(line, n);
   }
   buf.close();
}

struct StringListSource : QObject {
   Q_SIGNAL void signal(const QStringList &);
   void operator()(const QStringList &data) { emit signal(data); }
   Q_OBJECT
};

int main(int argc, char *argv[]) {
   QApplication app(argc, argv);
   QListView view;
   QStringListModel model;
   StringListSource signal;
   QObject::connect(&signal, &StringListSource::signal, &model, &QStringListModel::setStringList);
   QtConcurrent::run([&signal]{
      QBuffer file;
      signal({"Generating Data..."});
      makeLines(file);
      signal({"Loading Data..."});
      QStringList lines;
      if (file.open(QIODevice::ReadOnly | QIODevice::Text))
         while (!file.atEnd())
            lines.append(QString::fromLatin1(file.readLine()));
      file.close();
      signal(lines);
   });
   view.setModel(&model);
   view.setUniformItemSizes(true);
   view.show();
   return app.exec();
}
#include "main.moc"
