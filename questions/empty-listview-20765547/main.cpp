#include <QtWidgets>

class ListView : public QListView {
   void paintEvent(QPaintEvent *e) {
      QListView::paintEvent(e);
      if (model() && model()->rowCount(rootIndex()) > 0) return;
      // The view is empty.
      QPainter p(this->viewport());
      p.drawText(rect(), Qt::AlignCenter, "No Items");
   }
public:
   ListView(QWidget* parent = 0) : QListView(parent) {}
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QWidget window;
   QFormLayout layout(&window);
   ListView view;
   QSpinBox spin;
   QStringListModel model;
   layout.addRow(&view);
   layout.addRow("Item Count", &spin);
   QObject::connect(&spin, (void (QSpinBox::*)(int))&QSpinBox::valueChanged,
                    [&](int value){
      QStringList list;
      for (int i = 0; i < value; ++i) list << QString("Item %1").arg(i);
      model.setStringList(list);
   });
   view.setModel(&model);
   window.show();
   return a.exec();
}
