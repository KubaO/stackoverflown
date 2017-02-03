// https://github.com/KubaO/stackoverflown/tree/master/questions/win-move-track-42019943
#include <QtWidgets>

class WindowOffset : public QObject {
   Q_OBJECT
   QPoint m_offset, m_ref;
   QPointer<QWidget> m_src, m_dst;
   void adjust(QEvent * event, const QPoint & delta = QPoint{}) {
      qDebug() << "ADJUST" << delta << event;
      m_dst->move(m_src->geometry().topRight() + m_offset + delta);
   }
protected:
   bool eventFilter(QObject *watched, QEvent *event) override {
#ifdef Q_OS_OSX
      if (watched == m_src.data()) {
         if (event->type() == QEvent::NonClientAreaMouseButtonPress) {
            m_ref = QCursor::pos();
            qDebug() << "ACQ" << m_ref << event;
         }
         else if (event->type() == QEvent::NonClientAreaMouseMove &&
                  static_cast<QMouseEvent*>(event)->buttons() == Qt::LeftButton) {
            auto delta = QCursor::pos() - m_ref;
            adjust(event, delta);
         }
      }
#endif
      if ((watched == m_src.data() &&
                (event->type() == QEvent::Move || event->type() == QEvent::Resize)) ||
               (watched == m_dst.data() && event->type() == QEvent::Show)) {
         if (event->type() == QEvent::Move)
            m_ref = QCursor::pos();
         adjust(event);
      }
      return false;
   }
public:
   WindowOffset(const QPoint & offset, QObject * parent = nullptr) :
      QObject{parent},
      m_offset{offset}
   {}
   WindowOffset(QWidget * src, QWidget * dst, const QPoint & offset, QObject * parent = nullptr) :
      WindowOffset{offset, parent}
   {
      src->installEventFilter(this);
      dst->installEventFilter(this);
      m_src = src;
      m_dst = dst;
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QLabel d1{"Source"}, d2{"Target"};
   WindowOffset offset{&d1, &d2, {200, 50}};
   for (auto d : {&d1, &d2}) {
      d->setMinimumSize(300,100);
      d->show();
   }
   return app.exec();
}

#include "main.moc"
