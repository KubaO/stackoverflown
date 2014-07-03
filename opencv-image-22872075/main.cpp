#include <QApplication>
#include <QBasicTimer>
#include <QImage>
#include <QPixmap>
#include <QGridLayout>
#include <QLabel>
#include <opencv2/opencv.hpp>

namespace Ui { struct UIQT {
  QLabel * image;
  void setupUi(QWidget * w) {
    QGridLayout * layout = new QGridLayout(w);
    layout->addWidget((image = new QLabel));
  }
}; }

class UIQT : public QWidget {
  Q_OBJECT
  QScopedPointer<Ui::UIQT> ui;
  QBasicTimer m_timer;
  cv::Size m_size;
  void timerEvent(QTimerEvent *);
public:
  UIQT(QWidget * parent = 0);
  ~UIQT();
  Q_SLOT void refreshImage();
};

QByteArray b;

void matDeleter(void* mat) { delete static_cast<cv::Mat*>(mat); }

static QImage imageFromMat(cv::Mat const& src) {
  Q_ASSERT(src.type() == CV_8UC3);
  cv::Mat * mat = new cv::Mat(src.cols,src.rows,src.type());
  cvtColor(src, *mat, CV_BGR2RGB);
  return QImage((uchar*)mat->data, mat->cols, mat->rows, mat->step,
                QImage::Format_RGB888, &matDeleter, mat);
}

static cv::Scalar randomScalar() {
  static cv::RNG rng(12345);
  return cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
}

static QPixmap pixmapFromMat(const cv::Mat & src) {
  QImage image(imageFromMat(src));
  return QPixmap::fromImage(image);
}

UIQT::UIQT(QWidget * parent) :
  QWidget(parent), ui(new Ui::UIQT),
  m_size(100, 100)
{
  ui->setupUi(this);
  m_timer.start(500, this);
  refreshImage();
}

UIQT::~UIQT() {}

void UIQT::timerEvent(QTimerEvent * ev) {
  if (ev->timerId() != m_timer.timerId()) return;
  refreshImage();
}

void UIQT::refreshImage() {
  cv::Mat mat(m_size, CV_8UC3, randomScalar());
  ui->image->setPixmap(pixmapFromMat(mat));
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  UIQT w;
  w.show();
  return app.exec();
}

#include "main.moc"
