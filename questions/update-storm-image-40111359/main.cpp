// https://github.com/KubaO/stackoverflown/tree/master/questions/update-storm-image-40111359
#include <QtWidgets>

class ImageSource : public QObject {
  Q_OBJECT
  QImage m_frame{640, 480, QImage::Format_ARGB32_Premultiplied};
  QBasicTimer m_timer;
  double m_period{};
  void timerEvent(QTimerEvent * event) override {
    if (event->timerId() != m_timer.timerId()) return;
    m_frame.fill(Qt::blue);
    QElapsedTimer t;
    t.start();
    QPainter p{&m_frame};
    p.setFont({"Helvetica", 48});
    p.setPen(Qt::white);
    p.drawText(m_frame.rect(), Qt::AlignCenter,
               QStringLiteral("Hello,\nWorld!\n%1").arg(
                 QTime::currentTime().toString(QStringLiteral("hh:mm:ss.zzz"))));
    auto const alpha = 0.001;
    m_period = (1.-alpha)*m_period + alpha*(t.nsecsElapsed()*1E-9);
    emit newFrame(m_frame, m_period);
  }
public:
  ImageSource() {
    m_timer.start(0, this);
  }
  Q_SIGNAL void newFrame(const QImage &, double period);
};

class Viewer : public QWidget {
  Q_OBJECT
  double m_framePeriod;
  QImage m_image;
  QImage m_scaledImage;
  void paintEvent(QPaintEvent *) override {
    qDebug() << "Waiting events" << d_ptr->postedEvents;
    QPainter p{this};
    if (m_image.isNull()) return;
    if (m_scaledImage.isNull() || m_scaledImage.size() != size())
      m_scaledImage = m_image.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    p.drawImage(0, 0, m_scaledImage);
    p.drawText(rect(), Qt::AlignTop | Qt::AlignLeft, QStringLiteral("%1 FPS").arg(1./m_framePeriod));
    if (true) QThread::msleep(250);
  }
public:
  Q_SLOT void setImage(const QImage & image, double period) {
    Q_ASSERT(QThread::currentThread() == thread());
    m_image = image;
    m_scaledImage = {};
    m_framePeriod = period;
    update();
  }
};

class Thread final : public QThread { public: ~Thread() { quit(); wait(); } };

int main(int argc, char ** argv) {
  QApplication app{argc, argv};
  Viewer viewer;
  viewer.setMinimumSize(200, 200);
  ImageSource source;
  Thread thread;
  QObject::connect(&source, &ImageSource::newFrame, &viewer, &Viewer::setImage);
  QObject::connect(&thread, &QThread::destroyed, [&]{ source.moveToThread(app.thread()); });
  source.moveToThread(&thread);
  thread.start();
  viewer.show();
  return app.exec();
}
#include "main.moc"
