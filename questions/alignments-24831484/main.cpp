// https://github.com/KubaO/stackoverflown/tree/master/questions/alignments-24831484
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

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
   QRectF rect{corner.x(), corner.y(), size, size};
   painter.drawText(rect, flags, text, boundingRect);
}

void drawText(QPainter & painter, const QPointF & point, Qt::Alignment flags,
              const QString & text, QRectF * boundingRect = {})
{
   drawText(painter, point.x(), point.y(), flags, text, boundingRect);
}

int main(int argc, char *argv[])
{
   QApplication a{argc, argv};
   QLabel label;
   QPicture pic;
   pic.setBoundingRect({-100, -100, 200, 200});
   QPainter p(&pic);
   const QPointF pt;

   p.drawEllipse(pt, 3, 3);
   p.setFont({"Helvetica", 40});
   p.setPen({128, 0, 0, 128});
   drawText(p, pt, Qt::AlignBottom, "_LB");
   drawText(p, pt, Qt::AlignVCenter, "_LC");
   drawText(p, pt, Qt::AlignTop, "_LT");
   p.setPen({0, 128, 0, 128});
   drawText(p, pt, Qt::AlignBottom | Qt::AlignHCenter, "MB");
   drawText(p, pt, Qt::AlignVCenter | Qt::AlignHCenter, "MC");
   drawText(p, pt, Qt::AlignTop | Qt::AlignHCenter, "MT");
   p.setPen({0, 0, 128, 128});
   drawText(p, pt, Qt::AlignBottom | Qt::AlignRight, "RB_");
   drawText(p, pt, Qt::AlignVCenter | Qt::AlignRight, "RC_");
   drawText(p, pt, Qt::AlignTop | Qt::AlignRight, "RT_");
   p.end();

   label.setPicture(pic);
   label.show();
   return a.exec();
}
