#include <QApplication>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QCheckBox>
#include <QBoxLayout>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QImage>
#include <QPainter>
#include <QColor>
#include <QAtomicInt>
#include <QMutex>
#include <QtConcurrent>
#include <QStandardPaths>
#include <algorithm>
#include <functional>
#include <valarray>

/// Provides random images. There may be more than one response per request.
class RandomImageSource : public QObject {
  Q_OBJECT
  int m_parallelism;
  bool m_auto;
  QNetworkAccessManager m_mgr;
  QSet<QNetworkReply*> m_replies;
  QList<QUrl> m_deferred;
  QRegularExpression m_imgTagRE, m_imgUrlRE;
  QUrl m_randomGallery;
  void get(const QUrl & url) {
    if (m_replies.count() < m_parallelism) {
      QNetworkRequest req(url);
      req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
      m_replies.insert(m_mgr.get(req));
    } else
      m_deferred << url;
  }
  void finishReply(QNetworkReply * reply) {
    m_replies.remove(reply);
    if (reply) reply->deleteLater();
    if (! m_deferred.isEmpty()) get(m_deferred.takeLast());
    while (m_deferred.isEmpty() && m_auto) get(m_randomGallery);
  }
  Q_SLOT void rsp(QNetworkReply * reply) {
    auto loc = reply->header(QNetworkRequest::LocationHeader);
    if (loc.isValid()) {
      get(loc.toUrl()); // redirect
    } else {
      auto ct = reply->header(QNetworkRequest::ContentTypeHeader).toString();
      if (ct.startsWith("text/html"))
        foreach (QUrl url, parseImageUrls(reply->readAll()))
          get(url);
      else if (ct.startsWith("image")) {
        auto img = QImage::fromData(reply->readAll());
        img.setText("filename", m_imgUrlRE.match(reply->url().toString()).captured(1));
        if (!img.isNull()) emit rspImage(img);
      }
    }
    finishReply(reply);
  }
  QList<QUrl> parseImageUrls(const QByteArray & html) {
    QList<QUrl> urls;
    auto it = m_imgTagRE.globalMatch(QString::fromUtf8(html));
    while (it.hasNext()) { auto match = it.next(); // get small images
      urls << QUrl("http:" + match.captured(1) + "s" + match.captured(2)); }
    return urls;
  }
public:
  RandomImageSource(QObject * parent = 0) : QObject (parent),
    m_parallelism(20), m_auto(false),
    m_imgTagRE("<img src=\"(//i\\.imgur\\.com/[^.]+)(\\.[^\"]+)\""),
    m_imgUrlRE("http://i\\.imgur\\.com/(.+)$"),
    m_randomGallery("http://imgur.com/gallery/random")
  {
    connect(&m_mgr, SIGNAL(finished(QNetworkReply*)), SLOT(rsp(QNetworkReply*)));
  }
  Q_SLOT void reqImages(int count) {
    while (count--) get(m_randomGallery);
  }
  Q_SIGNAL void rspImage(const QImage &);
  bool automatic() const { return m_auto; }
  Q_SLOT void setAutomatic(bool a) { if ((m_auto = a)) finishReply(0); }
  int parallelism() const { return m_parallelism; }
  Q_SLOT void setParallelism(int p) { m_parallelism = p; if (m_auto) finishReply(0); }
};

/// Stores images on disk, and loads them in the background.
class ImageStorage : public QObject {
  Q_OBJECT
  QString const m_path;
public:
  ImageStorage() :
    m_path(QStandardPaths::writableLocation(QStandardPaths::DataLocation)
           + "/images/")
  { QDir().mkpath(m_path); }
  Q_SLOT void addImage(const QImage & img) {
    QString path = img.text("filename");
    if (path.isEmpty()) return;
    path.prepend(m_path);
    QtConcurrent::run([img, path]{ img.save(path); });
  }
  Q_SLOT void retrieveAll() {
    QString const path = m_path;
    QtConcurrent::run([this, path] {
      QStringList const images = QDir(path).entryList(QDir::Files);
      foreach (QString image, images) QtConcurrent::run([this, image, path] {
        QImage img; if (img.load(path + image)) emit retrieved(img);
      });
    });
  }
  Q_SIGNAL void retrieved(const QImage &);
};

/// A memory database of images. Finds best match to a given image.
class ImageDatabase : public QObject {
  Q_OBJECT
  typedef std::valarray<qreal> Props;
  typedef QPair<QImage, Props> ImageProps;
  QMutex mutable m_mutex;
  QList<ImageProps> m_images;
  static void inline addProps(Props & p, int i, QRgb rgb) {
    QColor const c = QColor::fromRgb(rgb);
    p[i+0] += c.redF(); p[i+1] += c.greenF(); p[i+2] += c.blueF();
  }
  static Props calcPropsFor(const QImage & img, int divs = 4) {
    Props props(0.0, 3 * divs * divs);
    std::valarray<int> counts(0, divs * divs);
    QSize div = img.size() / divs;
    for (int y = 0; y < img.height(); ++y)
      for (int x = 0; x < img.width(); ++x) {
        int slice = x/div.width() + (y*divs/div.height());
        if (slice >= divs*divs) continue;
        addProps(props, slice*3, img.pixel(x, y));
        counts[slice] ++;
      }
    for (size_t i = 0; i < props.size(); ++i) props[i] /= counts[i/3];
    return props;
  }
public:
  Q_SIGNAL void newImageCount(int);
  Q_SLOT void addImage(const QImage & img) {
    QtConcurrent::run([this, img]{
      Props props = calcPropsFor(img);
      QMutexLocker lock(&m_mutex);
      m_images << qMakePair(img, props);
      int count = m_images.count();
      lock.unlock();
      emit newImageCount(count);
    });
  }
  ImageProps bestMatchFor(const QImage & img, int randLog2) const {
    QMutexLocker lock(&m_mutex);
    QList<ImageProps> const images = m_images;
    lock.unlock();
    Props const props = calcPropsFor(img);
    typedef QPair<qreal, const ImageProps *> Match;
    QList<Match> matches; matches.reserve(images.size());
    std::transform(images.begin(), images.end(), std::back_inserter(matches),
                   [props](const ImageProps & prop){
      return qMakePair(pow(props - prop.second, 2).sum(), &prop);
    });
    std::sort(matches.begin(), matches.end(),
              [](Match a, Match b) { return b.first < a.first; });
    randLog2 = 1<<randLog2;
    return *(matches.end()-randLog2+qrand()%randLog2)->second;
  }
};

QImage getMosaic(QImage img, const ImageDatabase & db, int size, int randLog2)
{
  QPainter p(&img);
  for (int y = 0; y < img.height(); y += size)
    for (int x = 0; x < img.width(); x += size) {
      QImage r = db.bestMatchFor(img.copy(x, y, size, size), randLog2).first
          .scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
      p.drawImage(x, y, r);
    }
  return img;
}

class MosaicGenerator : public QObject {
  Q_OBJECT
  QPointer<ImageDatabase> m_db;
  int m_size, m_randLog2;
  QAtomicInt m_busy;
  QImage m_image;
  void update() {
    if (m_image.isNull() || m_busy.fetchAndAddOrdered(1)) return;
    QImage image = m_image;
    QtConcurrent::run([this, image]{ while (true) {
        emit hasMosaic(getMosaic(image, *m_db, m_size, m_randLog2));
        if (m_busy.testAndSetOrdered(1, 0)) return;
        m_busy.fetchAndStoreOrdered(1);
      }});
  }
public:
  MosaicGenerator(ImageDatabase * db) : m_db(db), m_size(16), m_randLog2(0) {}
  Q_SLOT void setImage(const QImage & img) { m_image = img; update(); }
  Q_SLOT void setSize(int s) { m_size = s; update(); }
  Q_SLOT void setRandLog2(int r) { m_randLog2 = r; update(); }
  Q_SIGNAL void hasMosaic(const QImage &);
};

class Window : public QWidget {
  Q_OBJECT
  bool m_showSource;
  QImage m_source, m_mosaic;
  QBoxLayout m_layout;
  QSlider m_parallelism, m_cellSize, m_randomness;
  QLabel m_imgCount, m_parCount, m_image;
  QPushButton m_add, m_load, m_toggle;
  MosaicGenerator m_gen;
  Q_SIGNAL void newSource(const QImage &);
  void updateImage() {
    const QImage & img = m_showSource ? m_source : m_mosaic;
    m_image.setPixmap(QPixmap::fromImage(img));
  }
public:
  Window(ImageDatabase * db, QWidget * parent = 0) : QWidget(parent),
    m_showSource(true), m_layout(QBoxLayout::TopToBottom, this),
    m_parallelism(Qt::Horizontal), m_cellSize(Qt::Horizontal),
    m_randomness(Qt::Horizontal), m_add("Fetch Images"),
    m_load("Open for Mosaic"), m_toggle("Toggle Mosaic"), m_gen(db)
  {
    QBoxLayout * row = new QBoxLayout(QBoxLayout::LeftToRight);
    row->addWidget(new QLabel("Images in DB:"));
    row->addWidget(&m_imgCount);
    row->addWidget(new QLabel("Fetch parallelism:"));
    row->addWidget(&m_parallelism);
    row->addWidget(&m_parCount);
    row->addWidget(&m_add);
    m_parallelism.setRange(1, 100);
    m_layout.addLayout(row);
    m_layout.addWidget(&m_image);
    row = new QBoxLayout(QBoxLayout::LeftToRight);
    row->addWidget(new QLabel("Cell Size:"));
    row->addWidget(&m_cellSize);
    row->addWidget(new QLabel("Randomness:"));
    row->addWidget(&m_randomness);
    m_cellSize.setRange(4, 64); m_cellSize.setTracking(false);
    m_randomness.setRange(0,6); m_randomness.setTracking(false);
    m_layout.addLayout(row);
    row = new QBoxLayout(QBoxLayout::LeftToRight);
    row->addWidget(&m_load);
    row->addWidget(&m_toggle);
    m_layout.addLayout(row);
    m_add.setCheckable(true);
    m_parCount.connect(&m_parallelism, SIGNAL(valueChanged(int)), SLOT(setNum(int)));
    connect(&m_add, SIGNAL(clicked(bool)), SIGNAL(reqAutoFetch(bool)));
    connect(&m_parallelism, SIGNAL(valueChanged(int)), SIGNAL(reqParallelism(int)));
    m_gen.connect(&m_cellSize, SIGNAL(valueChanged(int)), SLOT(setSize(int)));
    m_gen.connect(&m_randomness, SIGNAL(valueChanged(int)), SLOT(setRandLog2(int)));
    m_parallelism.setValue(20);
    m_cellSize.setValue(16);
    m_randomness.setValue(4);
    connect(&m_load, &QPushButton::clicked, [this]{
      QString file = QFileDialog::getOpenFileName(this);
      QtConcurrent::run([this, file]{
        QImage img; if (!img.load(file)) return;
        emit newSource(img);
      });
    });
    connect(this, &Window::newSource, [this](const QImage &img){
      m_source = m_mosaic = img; updateImage(); m_gen.setImage(m_source);
    });
    connect(&m_gen, &MosaicGenerator::hasMosaic, [this](const QImage &img){
      m_mosaic = img; updateImage();
    });
    connect(&m_toggle, &QPushButton::clicked, [this]{
      m_showSource = !m_showSource; updateImage();
    });
  }
  Q_SLOT void setImageCount(int n) { m_imgCount.setNum(n); }
  Q_SIGNAL void reqAutoFetch(bool);
  Q_SIGNAL void reqParallelism(int);
};

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  a.setOrganizationDomain("stackoverflow.com");
  a.setApplicationName("so-photomosaic");
  RandomImageSource src;
  ImageDatabase db;
  ImageStorage stg;
  Window ui(&db);
  db.connect(&src, SIGNAL(rspImage(QImage)), SLOT(addImage(QImage)));
  stg.connect(&src, SIGNAL(rspImage(QImage)), SLOT(addImage(QImage)));
  db.connect(&stg, SIGNAL(retrieved(QImage)), SLOT(addImage(QImage)));
  ui.connect(&db, SIGNAL(newImageCount(int)), SLOT(setImageCount(int)));
  src.connect(&ui, SIGNAL(reqAutoFetch(bool)), SLOT(setAutomatic(bool)));
  src.connect(&ui, SIGNAL(reqParallelism(int)), SLOT(setParallelism(int)));
  stg.retrieveAll();
  ui.show();
  return a.exec();
}

#include "main.moc"
