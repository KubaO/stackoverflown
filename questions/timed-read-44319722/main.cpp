// https://github.com/KubaO/stackoverflown/tree/master/questions/timed-read-44319722
#include <QtWidgets>

class PeriodicReader : public QObject {
   Q_OBJECT
   QTimer m_timer{this};
   QFile m_file{this};
   void readLine() {
      if (m_file.atEnd()) {
         m_timer.stop();
         return;
      }
      emit newLine(m_file.readLine());
   }
public:
   explicit PeriodicReader(QObject * parent = {}) : QObject(parent) {
      connect(&m_timer, &QTimer::timeout, this, &PeriodicReader::readLine);
   }
   void load(const QString & fileName) {
      m_file.close(); // allow re-opening of the file
      m_file.setFileName(fileName);
      if (m_file.open(QFile::ReadOnly | QFile::Text)) {
         readLine();
         m_timer.start(300); // 0.3s interval
      }
   }
   Q_SIGNAL void newLine(const QByteArray &);
};

QString lineToString(QByteArray line) {
   while (line.endsWith('\n') || line.endsWith('\r'))
      line.chop(1);
   return QString::fromUtf8(line);
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};

   QWidget window;
   QVBoxLayout layout{&window};
   QPushButton load{"Load"};
   QPlainTextEdit edit;
   layout.addWidget(&load);
   layout.addWidget(&edit);
   window.show();

   PeriodicReader reader;
   QObject::connect(&load, &QPushButton::clicked, [&]{
      auto name = QFileDialog::getOpenFileName(&window);
      if (!name.isEmpty()) {
         edit.clear(); // allow re-opening of the file
         reader.load(name);
      }
   });
   QObject::connect(&reader, &PeriodicReader::newLine, &edit,
                    [&](const QByteArray & line){ edit.appendPlainText(lineToString(line)); });

   return app.exec();
}
#include "main.moc"
