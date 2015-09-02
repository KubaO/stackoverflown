#include <QApplication>
#include <QLabel>
#include <QPainter>
#include <QPicture>
#include <QDebug>

void drawText(QPainter & painter, qreal x, qreal y, Qt::Alignment flags,
              const QString & text, QRectF * boundingRect = 0)
{
   const qreal size = 32767.0;
   QPointF corner(x, y - size);
   if (flags & Qt::AlignHCenter) corner.rx() -= size/2.0;
   else if (flags & Qt::AlignRight) corner.rx() -= size;
   if (flags & Qt::AlignVCenter) corner.ry() += size/2.0;
   else if (flags & Qt::AlignTop) corner.ry() += size;
   else flags |= Qt::AlignBottom;
   QRectF rect(corner, QSizeF(size, size));
   painter.drawText(rect, flags, text, boundingRect);
}

void drawText(QPainter & painter, const QPointF & point, Qt::Alignment flags,
              const QString & text, QRectF * boundingRect = 0)
{
   drawText(painter, point.x(), point.y(), flags, text, boundingRect);
}

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QLabel l;
   QPicture pic;
   pic.setBoundingRect(QRect(-100, -100, 200, 200));
   QPainter p(&pic);
   QPointF pt;

   p.drawEllipse(pt, 3, 3);
   p.setFont(QFont("Helvetica", 40));
   p.setPen(QColor(128, 0, 0, 128));
   drawText(p, pt, Qt::AlignBottom, "_LB");
   drawText(p, pt, Qt::AlignVCenter, "_LC");
   drawText(p, pt, Qt::AlignTop, "_LT");
   p.setPen(QColor(0, 128, 0, 128));
   drawText(p, pt, Qt::AlignBottom | Qt::AlignHCenter, "MB");
   drawText(p, pt, Qt::AlignVCenter | Qt::AlignHCenter, "MC");
   drawText(p, pt, Qt::AlignTop | Qt::AlignHCenter, "MT");
   p.setPen(QColor(0, 0, 128, 128));
   drawText(p, pt, Qt::AlignBottom | Qt::AlignRight, "RB_");
   drawText(p, pt, Qt::AlignVCenter | Qt::AlignRight, "RC_");
   drawText(p, pt, Qt::AlignTop | Qt::AlignRight, "RT_");
   p.end();

   l.setPicture(pic);
   l.show();
   return a.exec();
}
