// https://github.com/KubaO/stackoverflown/tree/master/questions/checkered-paint-42205180
#include <QtWidgets>

QPixmap checkers(const QSizeF & rectSize, const QSize & pixmapSize) {
   QPixmap pixmap{pixmapSize};
   QPainter painter{&pixmap};
   painter.setPen(Qt::NoPen);
   const QColor colors[] = {Qt::white, Qt::black};
   QPointF pos{};
   bool oddRow{}, color{};
   while (pos.y() < pixmap.height()) {
      painter.setBrush(colors[color ? 1 : 0]);
      painter.drawRect({pos, rectSize});
      color = !color;
      pos.setX(pos.x() + rectSize.width());
      if (pos.x() >= pixmap.width()) {
         pos.setX({});
         pos.setY(pos.y() + rectSize.height());
         oddRow = !oddRow;
         color = oddRow;
      }
   }
   return pixmap;
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QLabel label;
   auto pix = checkers({1, 1}, {60, 30});
   label.resize(200, 200);
   label.setAlignment(Qt::AlignCenter);
   label.setPixmap(pix);
   label.show();
   return app.exec();
}
