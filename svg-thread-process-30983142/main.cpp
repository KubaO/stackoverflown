#include <QGraphicsView>
#include <QGraphicsSvgItem>
#include <QGraphicsSceneMouseEvent>
#include <QFileDialog>
#include <QSvgRenderer>
#include <QDomDocument>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QThread>
#include <QApplication>

struct Thread : public QThread { using QThread::sleep; }; // Needed for Qt 4 only

class RendererGenerator {
   QString m_fileName;
   void process(QDomDocument &) {
      Thread::sleep(3); /* let's pretend we process the DOM for a long time here */
   }
   QByteArray generate(const QByteArray & data) {
      QDomDocument dom;
      dom.setContent(data);
      process(dom);
      return dom.toByteArray();
   }
public:
   typedef QSvgRenderer * result_type;
   RendererGenerator(const QString & fileName) : m_fileName(fileName) {}
   QSvgRenderer * operator()() {
      QFile file(m_fileName);
      if (file.open(QIODevice::ReadOnly)) {
         QByteArray data = file.readAll();
         QScopedPointer<QSvgRenderer> renderer(new QSvgRenderer);
         renderer->load(generate(data));
         renderer->moveToThread(0);
         return renderer.take();
      }
      return 0;
   }
};

class UserSvgItem : public QGraphicsSvgItem {
   Q_OBJECT
   QSvgRenderer m_spinRenderer, * m_lastRenderer;
   QScopedPointer<QSvgRenderer> m_renderer;
   QFuture<QSvgRenderer*> m_future;
   QFutureWatcher<QSvgRenderer*> m_watcher;
   QGraphicsView * aView() const {
      QList<QGraphicsView*> views = scene()->views();
      return views.isEmpty() ? 0 : views.first();
   }
   Q_SLOT void update() { QGraphicsSvgItem::update(); }
   void mousePressEvent(QGraphicsSceneMouseEvent * event) {
      if (event->button() == Qt::LeftButton) askForFile();
   }
   void setRenderer(QSvgRenderer * renderer) {
      if (m_lastRenderer) disconnect(m_lastRenderer, SIGNAL(repaintNeeded()), this, SLOT(update()));
      setSharedRenderer(renderer);
      m_lastRenderer = renderer;
      connect(renderer, SIGNAL(repaintNeeded()), SLOT(update()));
      if (aView()) aView()->centerOn(this);
   }
   void askForFile() {
      QFileDialog * dialog = new QFileDialog(aView());
      connect(dialog, SIGNAL(fileSelected(QString)), SLOT(loadFile(QString)));
      dialog->setAcceptMode(QFileDialog::AcceptOpen);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
   }
   Q_SLOT void loadFile(const QString & file) {
      if (m_future.isRunning()) return;
      setRenderer(&m_spinRenderer);
      m_future = QtConcurrent::run(RendererGenerator(file));
      m_watcher.setFuture(m_future);
   }
   Q_SLOT void rendererReady() {
      m_renderer.reset(m_future.result());
      m_renderer->moveToThread(thread());
      setRenderer(m_renderer.data());
   }
public:
   UserSvgItem(const QString & fileName = QString(), QGraphicsItem *parent = 0) :
      QGraphicsSvgItem(fileName, parent), m_lastRenderer(0) {
      connect(&m_watcher, SIGNAL(finished()), SLOT(rendererReady()));
      setFlags(QGraphicsItem::ItemClipsToShape);
      setCacheMode(QGraphicsItem::NoCache);
   }
   void setWaitAnimation(const QByteArray & data) { m_spinRenderer.load(data); }
};

namespace {
   const char svgCircle[] =
      "<svg height=\"100\" width=\"100\"><circle cx=\"50\" cy=\"50\" r=\"40\" stroke=\"black\" stroke-width=\"3\" fill=\"red\" /></svg>";
   const char svgRectangle[] =
      "<svg width=\"400\" height=\"110\"><rect width=\"300\" height=\"100\" style=\"fill:rgb(0,0,255);stroke-width:3;stroke:rgb(0,0,0)\"></svg>";
   const char svgThrobber[] =
      "<svg width=\"16\" height=\"16\" viewBox=\"0 0 300 300\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\"><path d=\"M 150,0 a 150,150 0 0,1 106.066,256.066 l -35.355,-35.355 a -100,-100 0 0,0 -70.711,-170.711 z\" fill=\"#3d7fe6\"><animateTransform attributeName=\"transform\" attributeType=\"XML\" type=\"rotate\" from=\"0 150 150\" to=\"360 150 150\" begin=\"0s\" dur=\"1s\" fill=\"freeze\" repeatCount=\"indefinite\" /></path></svg>";

   void write(const char * str, const QString & fileName) {
      QFile out(fileName);
      if (out.open(QIODevice::WriteOnly | QIODevice::Truncate)) out.write(str);
   }
}

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   write(svgRectangle, "rectangle.svg"); // Put svg resources into the working directory
   write(svgCircle, "circle.svg");

   QGraphicsScene scene;
   UserSvgItem item("circle.svg");
   QGraphicsView view(&scene);
   scene.addItem(&item);
   item.setWaitAnimation(QByteArray::fromRawData(svgThrobber, sizeof(svgThrobber)-1));
   view.show();

   return app.exec();
}

#include "main.moc"
