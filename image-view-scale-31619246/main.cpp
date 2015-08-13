#include <QtWidgets>
#include <QtNetwork>

class FragTreeViewer : public QWidget {
   QGridLayout m_layout;
   QScrollArea m_area;
   QLabel m_imageLabel, m_scaleLabel;
   QPushButton m_zoomOut, m_zoomIn;
   double m_scaleFactor;
public:
   void setImage(const QImage & img) {
      m_scaleFactor = 1.0;
      m_imageLabel.setPixmap(QPixmap::fromImage(img));
      scaleImage(1.0);
   }
   FragTreeViewer() :
      m_layout(this), m_zoomOut("Zoom Out"), m_zoomIn("Zoom In"),
      m_scaleFactor(1.0) {
      m_layout.addWidget(&m_area, 0, 0, 1, 3);
      m_layout.addWidget(&m_zoomOut, 1, 0);
      m_layout.addWidget(&m_scaleLabel, 1, 1);
      m_layout.addWidget(&m_zoomIn, 1, 2);
      m_area.setWidget(&m_imageLabel);
      m_imageLabel.setScaledContents(true);
      connect(&m_zoomIn, &QPushButton::clicked, [this]{ scaleImage(1.1); });
      connect(&m_zoomOut, &QPushButton::clicked, [this]{ scaleImage(1.0/1.1); });
   }
   void scaleImage(double factor) {
      m_scaleFactor *= factor;
      m_scaleLabel.setText(QStringLiteral("%1%").arg(m_scaleFactor*100, 0, 'f', 1));
      QSize size = m_imageLabel.pixmap()->size() * m_scaleFactor;
      m_imageLabel.resize(size);
   }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   FragTreeViewer viewer;
   QNetworkAccessManager mgr;
   QScopedPointer<QNetworkReply> rsp(
            mgr.get(QNetworkRequest(QUrl("http://i.imgur.com/ikwUmUV.jpg"))));
   QObject::connect(rsp.data(), &QNetworkReply::finished, [&]{
      if (rsp->error() == QNetworkReply::NoError)
         viewer.setImage(QImage::fromData(rsp->readAll()));
   });
   viewer.show();
   return a.exec();
}
