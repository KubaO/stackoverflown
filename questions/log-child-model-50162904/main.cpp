// https://github.com/KubaO/stackoverflown/tree/master/questions/log-child-model-50162904
#include <QtWidgets>

int main(int argc, char *argv[]) {
   QApplication app{argc, argv};

   QPushButton parent{"Click to close"};
   QPlainTextEdit log{&parent};
   log.setWindowFlag(Qt::Window);
   QTimer timer;
   timer.start(100);
   QObject::connect(&timer, &QTimer::timeout, []{
      qDebug() << QDateTime::currentDateTimeUtc();
   });
   QObject::connect(&parent, &QPushButton::clicked, &parent, &QWidget::close);
   parent.setMinimumSize(320, 240);
   log.setMinimumSize(640, 480);
   parent.show();
   log.show();
   static QPointer<QPlainTextEdit> staticLog = &log;
   qInstallMessageHandler([](auto, auto &, auto &msg){
      if (staticLog) staticLog->appendPlainText(msg);
   });
   return app.exec();
}
