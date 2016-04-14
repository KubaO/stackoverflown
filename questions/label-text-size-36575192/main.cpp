// https://github.com/KubaO/stackoverflown/tree/master/questions/label-text-size-36575192
#include <QtWidgets>

class LabelStretcher : public QObject {
   Q_OBJECT
public:
   LabelStretcher(QObject * parent = 0) : QObject(parent) {
      apply(qobject_cast<QLabel*>(parent));
   }
   void apply(QLabel * label) { if (label) label->installEventFilter(this); }
protected:
   bool eventFilter(QObject * obj, QEvent * ev) Q_DECL_OVERRIDE {
      if (ev->type() != QEvent::Resize) return false;
      auto label = qobject_cast<QLabel*>(obj);
      if (!label || label->text().isEmpty() || !label->hasScaledContents()) return false;
      qDebug() << "pre: " << label->minimumSizeHint() << label->sizeHint() << label->size();

      static auto dSize = [](const QSize & inner, const QSize & outer) -> int {
         auto dy = inner.height() - outer.height();
         auto dx = inner.width() - outer.width();
         return std::max(dx, dy);
      };
      static auto f = [](qreal fontSize, QLabel * label) -> qreal {
         auto font = label->font();
         font.setPointSizeF(fontSize);
         label->setFont(font);
         auto d = dSize(label->sizeHint(), label->size());
         qDebug() << "f:" << fontSize << "d" << d;
         return d;
      };
      static auto df = [](qreal fontSize, QLabel * label) -> qreal {
         if (fontSize < 1.0) fontSize = 1.0;
         return f(fontSize + 0.5, label) - f(fontSize - 0.5, label);
      };

      // Newton's method
      auto font = label->font();
      auto fontSize = font.pointSizeF();
      int i;
      for (i = 0; i < 5; ++i) {
         auto d = df(fontSize, label);
         qDebug() << "d:" << d;
         if (d < 0.1) break;
         fontSize -= f(fontSize, label)/d;
      }
      font.setPointSizeF(fontSize);
      label->setFont(font);
      qDebug() << "post:" << i << label->minimumSizeHint() << label->sizeHint() << label->size();
      return false;
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QLabel label{"Hello There!"};
   label.setScaledContents(true);
   label.show();
   LabelStretcher stretch(&label);
   return app.exec();
}

#include "main.moc"
