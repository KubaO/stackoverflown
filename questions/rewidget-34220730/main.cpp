// https://github.com/KubaO/stackoverflown/tree/master/questions/rewidget-34220730
#include <QtWidgets>

class MyWidgetPrivate {
public:
  bool qtRendering;
  // your data members etc.
  MyWidgetPrivate(bool qtRendering) :
    qtRendering(qtRendering)
  {}
};

class MyWidget : public QWidget {
  Q_OBJECT
  Q_DECLARE_PRIVATE(MyWidget)
  QScopedPointer<MyWidgetPrivate> const d_ptr;

  QPaintEngine *paintEngine() const {
    Q_D(const MyWidget);
    return d->qtRendering ? QWidget::paintEngine() : nullptr;
  }

  MyWidget(QScopedPointer<MyWidgetPrivate> & data, QWidget * parent, bool qtRendering) :
    QWidget(parent),
    d_ptr(data.take())
  {
     d_ptr->qtRendering = qtRendering;
  }
public:
  MyWidget(QWidget * parent, bool qtRendering) :
    QWidget(parent),
    d_ptr(new MyWidgetPrivate(qtRendering))
  {}

  void setQtRendering(bool qtRendering) {
    if (qtRendering == d_ptr->qtRendering) return;
    auto geom = geometry();
    auto parent = this->parentWidget();
    auto & d = const_cast<QScopedPointer<MyWidgetPrivate>&>(d_ptr);
    QScopedPointer<MyWidgetPrivate> pimpl(d.take());
    this->~MyWidget(); // destroy in place
    new (this) MyWidget(pimpl, parent, qtRendering); // reconstruct in place
    setGeometry(geom);
  }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   return app.exec();
}

