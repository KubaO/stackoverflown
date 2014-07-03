#include <QCoreApplication>
#include <QSharedPointer>
#include <QImage>
#include <QMap>
#include <QDebug>
#include <functional>
#include <core/types_c.h>

// Stand-in for OpenCV
namespace cv {
  class Mat {
    QSharedPointer<uchar> data_ptr;
  public:
    uchar * data;
    int rows, cols;
    Mat(int rows_, int cols_) :
        data_ptr(new uchar[4*rows*cols], [=](uchar * p){ delete [] p; }),
        data(data_ptr.data()), rows(rows_), cols(cols_) {}
    Mat() : rows(0), cols(0) {}
  };
}
void readFrame(cv::Mat & frame) {
    frame = cv::Mat(32, 32);
}

QString name(const uchar* ptr) {
    static QMap<const uchar*, QString> pointerNames;
    QMap<const uchar*, QString>::iterator it = pointerNames.find(ptr);
    if (it != pointerNames.end()) return it.value();
    return pointerNames.insert(ptr, QString("ptr#%1").arg(pointerNames.size())).value();
}

class MatImage {
    cv::Mat m_mat;
public:
    cv::Mat & mat() { return m_mat; }
    const cv::Mat & mat() const { return m_mat; }
    const cv::Mat & constMat() const { return m_mat; }
    QImage image() const { return QImage(m_mat.data, m_mat.rows, m_mat.cols, QImage::Format_RGB32); }
};
Q_DECLARE_METATYPE(MatImage)

void matDeleter(void* mat) { delete static_cast<cv::Mat*>(mat); }

class Test : public QObject {
    Q_OBJECT
    MatImage img;
public:
    Test() {
        readFrame(img.mat());
        qDebug() << "raw buffer" << name(img.mat().data);
    }
    ~Test() { qDebug() << __FUNCTION__; }
    Q_SLOT void matSlot(const MatImage & img) {
        qDebug() << __FUNCTION__ << name(img.image().constScanLine(0));
    }
    Q_SLOT void imgSlot(const QImage & img) {
        qDebug() << __FUNCTION__ << name(img.constScanLine(0));
    }
    Q_SIGNAL void matSignal(const MatImage &);
    Q_SIGNAL void imgSignal(const QImage &);
    Q_SLOT void sendImage() {
        QImage qi(img.image());
        qDebug() << "QImage buffer before" << name(qi.constScanLine(0));
        emit matSignal(img);
        emit imgSignal(img.image());
        qDebug() << "QImage buffer after" << name(qi.constScanLine(0));

        emit imgSignal(QImage(img.mat().data, img.mat().rows, img.mat().cols, QImage::Format_ARGB32, matDeleter, &img));

    }
    void loop(Qt::ConnectionType conn) {
        connect(this, SIGNAL(matSignal(MatImage)), SLOT(matSlot(MatImage)), conn);
        connect(this, SIGNAL(imgSignal(QImage)), SLOT(imgSlot(QImage)), conn);
    }
};

int main(int argc, char *argv[])
{
    qRegisterMetaType<MatImage>();
    QCoreApplication app(argc, argv);
    Test t1;
    qDebug("** Direct Connection **");
    t1.loop(Qt::AutoConnection);
    t1.sendImage();
    t1.disconnect();
    qDebug("** Queued Connection **");
    t1.loop(Qt::QueuedConnection);
    t1.sendImage();
    t1.disconnect();
    app.processEvents();
    return 0;
}


#include "main.moc"
