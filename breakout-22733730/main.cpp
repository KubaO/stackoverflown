#include <QApplication>
#include <QBasicTimer>
#include <QLabel>
#include <QGridLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <algorithm>

// References:
// http://codeincomplete.com/posts/2011/5/14/javascript_pong/part4/
// https://github.com/jakesgordon/javascript-breakout/blob/master/game.js
// http://paulbourke.net/geometry/pointlineplane/

typedef QList<QRect> QRectList;

static QColor randomColor() {
  return QColor(qrand() % 256, qrand() % 256, qrand() % 256);
}

static void setColor(QWidget * w, const QColor & color) {
  QPalette p(w->palette());
  p.setColor(QPalette::Normal, QPalette::Window, color);
  p.setColor(QPalette::Inactive, QPalette::Window, color);
  w->setPalette(p);
  w->setAutoFillBackground(true);
}

static QPointF movedBy(QPoint p, QVector2D vel) {
  return QPoint(p.x() + vel.x(), p.y() + vel.y());
}

static QVector2D manhattanNormalize(QVector2D v)
{
  return v / qMax(qAbs(v.x()), qAbs(v.y()));
}

static QList<int> bounce(QPoint & p0, QVector2D vel, QSize size, const QRectList & rectsIn)
{
  QList<int> touched;
#if 0
  QList<QPair<int, QRect> > rects;
  QRect swept = QRect(p0, size).united(QRect(movedBy(p0, vel), size));
  for (int i = 0; i < rectsIn.count(); ++i)
    if (rectsIn.at(i).intersects(swept)) rects << qMakePair(i, rectsIn.at(i));
  QVector2D p(p0.x(), p0.y());
  QVector2D d(td.manhattanNormalize(vel));
  while (vel.lengthSquared() > d.lengthSquared()) {

  }

#endif
  return touched;
}

class GameWindow : public QWidget {
  Q_OBJECT
  QBasicTimer m_update;
  QGridLayout m_topLayout, m_gameLayout;
  QWidgetList m_bricks;
  QLabel m_timer, m_ball;
  QFrame m_paddle;
  int m_prevPaddleX;
  QVector2D m_paddleVel, m_ballVel;
  const int m_R, m_C;
  enum { Serve, Play, Over } m_state;
  void newBricks() {
    while (!m_bricks.isEmpty()) delete m_bricks.takeLast();
    for (int r = 0; r < m_R; ++r)
      for (int c = 0; c < m_C; ++c) {
        QFrame * brick = new QFrame;
        brick->setFixedSize(50, 25);
        setColor(brick, randomColor());
        m_bricks << brick;
        m_gameLayout.addWidget(brick, r, c);
      }
    m_ball.raise();
  }
  void mouseMoveEvent(QMouseEvent * ev) {
    QRect field = m_gameLayout.geometry();
    int x = qMin(field.x()+field.width() - m_paddle.width(),
                 qMax(field.x(), ev->x() - m_paddle.width()/2));
    m_paddle.move(x, field.y() + field.height() - m_paddle.height());
    m_paddleVel.setX(m_prevPaddleX >= 0 ? x-m_prevPaddleX : 0);
    m_prevPaddleX = x;
    if (m_state == Serve) m_ball.move(x+m_paddle.width()/2 - m_ball.width()/2,
                                      m_paddle.y() - m_ball.height());
  }
  void mousePressEvent(QMouseEvent *) {
    if (m_state == Serve) {
      m_ballVel = QVector2D(m_paddleVel.x(), -5);
      m_update.start(1000/50, Qt::PreciseTimer, this);
      m_state = Play;
    }
    else if (m_state == Over) {
      newBricks();
      m_state = Serve;
    }
  }
  void timerEvent(QTimerEvent * ev) {
    if (ev->timerId() != m_update.timerId()) return;
    QRect field = m_gameLayout.geometry();
    qDebug() << field;
    return;
    QRectList rects;
    foreach (QWidget * w, m_bricks) rects << w->geometry();
    rects << m_paddle.geometry();
    rects << QRect(0, 0, field.left(), height())      // [
          << QRect(field.right(), 0, 100, height())   // ]
          << QRect(0, 0, width(), field.top())        // ^^
          << QRect(0, field.bottom(), width(), 100);  // vv
    QPoint p = m_ball.pos();
    QList<int> struck = bounce(p, m_ballVel, m_ball.size(), rects);
    std::sort(struck.begin(), struck.end(), std::greater<int>());
    foreach (int index, struck)
      if (index < m_bricks.count())
        delete m_bricks.takeAt(index);
    m_ball.move(p);
  }

public:
  GameWindow(QWidget * parent = 0) : QWidget(parent),
    m_topLayout(this), m_ball(this), m_paddle(this),
    m_prevPaddleX(-1), m_R(8), m_C(16), m_state(Serve)
  {
    setMouseTracking(true);
    m_gameLayout.setSpacing(1);
    m_topLayout.addLayout(&m_gameLayout, 0, 0);
    newBricks();
    m_gameLayout.addItem(new QSpacerItem(0, 400), m_R, 0, 1, m_C);
    m_topLayout.addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum,
                                        QSizePolicy::MinimumExpanding),
                        1, 0);
    setColor(&m_paddle, Qt::darkBlue);
    m_paddle.setFixedSize(80, 8);
    QPixmap bp(16, 16);
    bp.fill(Qt::transparent);
    QPainter p(&bp);
    p.setBrush(Qt::darkGreen);
    p.drawEllipse(bp.rect().adjusted(0,0,-1,-1));
    m_ball.setPixmap(bp);
    m_ball.setFixedSize(bp.size());
  }
};

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  GameWindow w;
  w.show();
  return a.exec();
}

#include "main.moc"
