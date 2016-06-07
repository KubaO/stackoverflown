// https://github.com/KubaO/stackoverflown/tree/master/questions/layout-stretch-triad-37680657
#include <QtWidgets>


struct Window : public QMainWindow {
   QWidget central;
   QHBoxLayout layout{&central};
   QListView view;
   QFrame frame;
   Window(const QString & title) {
      setWindowTitle(title);
      setCentralWidget(&central);
      view.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      frame.setLineWidth(3);
      frame.setFrameStyle(QFrame::Box);
      resize(500, 200);
      show();
   }
};

struct Window1 : Window {
   Window1() : Window("W1") {
      layout.addWidget(&view, 1);
      layout.addWidget(&frame, 2);
   }
};

struct Window2 : Window {
   QSplitter splitter;
   Window2() : Window("W2") {
      layout.addWidget(&splitter);
      splitter.addWidget(&view);
      splitter.addWidget(&frame);
      splitter.setStretchFactor(0, 1);
      splitter.setStretchFactor(1, 2);
   }
   ~Window2() { frame.setParent(0); view.setParent(0); }
};

struct Window3 : Window {
   QSplitter splitter;
   QWidget leftWidget;
   QVBoxLayout leftLayout{&leftWidget};
   Window3() : Window("W3") {
      layout.addWidget(&splitter);
      splitter.addWidget(&leftWidget);
      splitter.addWidget(&frame);
      splitter.setStretchFactor(0, 1);
      splitter.setStretchFactor(1, 2);
      leftLayout.setMargin(0);
      leftLayout.addWidget(&view);
   }
   ~Window3() { frame.setParent(0); view.setParent(0); }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Window1 w1;
   Window2 w2;
   Window3 w3;
   w2.move(w1.pos() + QPoint(0, 75));
   w3.move(w2.pos() + QPoint(0, 75));
   return app.exec();
}
