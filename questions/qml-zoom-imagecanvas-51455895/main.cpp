// https://github.com/KubaO/stackoverflown/tree/master/questions/qml-zoom-imagecanvas-51455895
#include <QtQuick>
#include <limits>

class ImageCanvas : public QQuickPaintedItem {
   Q_OBJECT
   Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)
   Q_PROPERTY(QRectF imageRect READ imageRect NOTIFY imageRectChanged)
   Q_PROPERTY(QPointF offset READ offset WRITE setOffset NOTIFY offsetChanged)
   Q_PROPERTY(double zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
   Q_PROPERTY(double renderTime READ renderTime NOTIFY renderTimeChanged)
   Q_PROPERTY(bool rectDraw READ rectDraw WRITE setRectDraw NOTIFY rectDrawChanged)
   Q_PROPERTY(QRectF sourceRect READ sourceRect NOTIFY sourceRectChanged)
   Q_PROPERTY(QRectF targetRect READ targetRect NOTIFY targetRectChanged)
public:
   ImageCanvas(QQuickItem *parent = {}) : QQuickPaintedItem(parent) {}

   QImage image() const { return mImage; }
   QRectF imageRect() const { return mImage.rect(); }
   void setImage(const QImage &image) {
      if (mImage != image) {
         auto const oldRect = mImage.rect();
         mImage = image;
         recalculate();
         emit imageChanged(mImage);
         if (mImage.rect() != oldRect)
            emit imageRectChanged(mImage.rect());
      }
   }
   Q_SIGNAL void imageChanged(const QImage &);
   Q_SIGNAL void imageRectChanged(const QRectF &);

   QPointF offset() const { return mOffset; }
   void setOffset(const QPointF &offset) {
      mOffset = offset;
      recalculate();
      emit offsetChanged(mOffset);
   }
   Q_SIGNAL void offsetChanged(const QPointF &);

   double zoom() const { return mZoom; }
   void setZoom(double zoom) {
      if (zoom != mZoom) {
         mZoom = zoom ? zoom : std::numeric_limits<float>::min();
         recalculate();
         emit zoomChanged(mZoom);
      }
   }
   Q_SIGNAL void zoomChanged(double);

   // **paint
   void paint(QPainter *p) override {
      QElapsedTimer timer;
      timer.start();
      if (mRectDraw) {
         p->drawImage(mTargetRect, mImage, mSourceRect);
      } else {
         p->scale(mZoom, mZoom);
         p->translate(-mOffset);
         p->drawImage(0, 0, mImage);
      }
      mRenderTime = timer.nsecsElapsed() * 1E-9;
      emit renderTimeChanged(mRenderTime);
   }
   double renderTime() const { return mRenderTime; }
   Q_SIGNAL void renderTimeChanged(double);

   bool rectDraw() const { return mRectDraw; }
   void setRectDraw(bool r) {
      if (r != mRectDraw) {
         mRectDraw = r;
         recalculate();
         emit rectDrawChanged(mRectDraw);
      }
   }
   Q_SIGNAL void rectDrawChanged(bool);
   QRectF sourceRect() const { return mSourceRect; }
   QRectF targetRect() const { return mTargetRect; }
   Q_SIGNAL void sourceRectChanged(const QRectF &);
   Q_SIGNAL void targetRectChanged(const QRectF &);

protected:
   void geometryChanged(const QRectF &, const QRectF &) override {
      recalculate();
   }

   // **private
private:
   QImage mImage;
   QPointF mOffset;
   double mZoom = 1.0;
   double mRenderTime = 0.;
   bool mRectDraw = true;
   QRectF mSourceRect;
   QRectF mTargetRect;

   static void moveBy(QRectF &r, const QPointF &o) {
      r = {r.x() + o.x(), r.y() + o.y(), r.width(), r.height()};
   }
   static void scaleBy(QRectF &r, qreal s) {
      r = {r.x() * s, r.y() * s, r.width() * s, r.height() * s};
   }
   void recalculate() {
      const auto oldTargetRect = mTargetRect;
      const auto oldSourceRect = mSourceRect;

      mTargetRect = {{}, mImage.size()};
      moveBy(mTargetRect, -mOffset);
      scaleBy(mTargetRect, mZoom);
      mTargetRect = mTargetRect.intersected({{}, size()});

      mSourceRect = mTargetRect;
      scaleBy(mSourceRect, 1.0/mZoom);
      moveBy(mSourceRect, mOffset);

      if (mTargetRect != oldTargetRect)
         emit targetRectChanged(mTargetRect);
      if (mSourceRect != oldSourceRect)
         emit sourceRectChanged(mSourceRect);
      update();
   }
};

QImage sampleImage() {
   QImage image(500, 500, QImage::Format_ARGB32_Premultiplied);
   QPainter painter(&image);
   for (int y = 0; y < image.height(); y += 50)
      for (int x = 0; x < image.width(); x += 50) {
         const QColor colour((x / 500.0) * 255, (y / 500.0) * 255, 0);
         painter.fillRect(x, y, 50, 50, colour);
      }
   return image;
}

int main(int argc, char *argv[])
{
   QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
   QGuiApplication app(argc, argv);

   qmlRegisterType<ImageCanvas>("App", 1, 0, "ImageCanvas");

   QQmlApplicationEngine engine;
   engine.rootContext()->setContextProperty("sampleImage", sampleImage());
   engine.load(QUrl("qrc:/main.qml"));

   return app.exec();
}

#include "main.moc"
