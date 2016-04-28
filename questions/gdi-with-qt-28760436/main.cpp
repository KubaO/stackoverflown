#include <QApplication>
#include <QGraphicsObject>
#include <QPropertyAnimation>
#include <QGraphicsView>
#include <QPainter>
#include <QPaintEvent>
#include <private/qwindowsbackingstore.h>
#include <windows.h>

class View : public QGraphicsView {
public:
   View(QWidget *parent = 0) : QGraphicsView(parent)
   {
      // This brings the original paint engine alive.
      QGraphicsView::paintEngine();
      //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
      setAttribute(Qt::WA_PaintOnScreen);
      setRenderHint(QPainter::Antialiasing);
   }
   QPaintEngine * paintEngine() const Q_DECL_OVERRIDE { return nullptr; }
   bool event(QEvent * event) Q_DECL_OVERRIDE {
      if (event->type() == QEvent::Paint) {
         bool result = QGraphicsView::event(event);
         paintOverlay();
         return result;
      }
      if (event->type() == QEvent::UpdateRequest) {
         bool result = QGraphicsView::event(event);
         paintOverlay();
         return result;
      }
      return QGraphicsView::event(event);
   }
   void resizeEvent(QResizeEvent *) {
       fitInView(-2, -2, 4, 4, Qt::KeepAspectRatio);
   }
   virtual void paintOverlay();
};

void View::paintOverlay()
{
   // We're called after the native painter has done its thing
   HWND hwnd = (HWND)viewport()->winId();
   HDC hdc = GetDC(hwnd);
   HBRUSH hbrGreen = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 255, 0));

   RECT rect;
   GetClientRect(hwnd, &rect);

   SetBkMode(hdc, TRANSPARENT);
   SelectObject(hdc, hbrGreen);
   Rectangle(hdc, 0, 0, rect.right, rect.bottom);

   SelectObject(hdc, GetStockObject(NULL_BRUSH));
   Ellipse(hdc, 50, 50, rect.right - 100, rect.bottom - 100);

   QString text("Test GDI Paint");
   SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
   TextOutW(hdc, width() / 2, height() / 2, (LPCWSTR)text.utf16(), text.size());

   DeleteObject(hbrGreen);
   ReleaseDC(hwnd, hdc);
}

class EmptyGraphicsObject : public QGraphicsObject
{
public:
    EmptyGraphicsObject() {}
    QRectF boundingRect() const { return QRectF(0, 0, 0, 0); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) {}
};

void setupScene(QGraphicsScene &s)
{
    QGraphicsObject * obj = new EmptyGraphicsObject;
    QGraphicsRectItem * rect = new QGraphicsRectItem(-1, 0.3, 2, 0.3, obj);
    QPropertyAnimation * anim = new QPropertyAnimation(obj, "rotation", &s);
    s.addItem(obj);
    rect->setPen(QPen(Qt::darkBlue, 0.1));
    anim->setDuration(2000);
    anim->setStartValue(0);
    anim->setEndValue(360);
    anim->setEasingCurve(QEasingCurve::InBounce);
    anim->setLoopCount(-1);
    anim->start();
}

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QGraphicsScene s;
   setupScene(s);
   View view;
   view.setScene(&s);
   view.show();
   return a.exec();
}
