// https://github.com/KubaO/stackoverflown/tree/master/questions/glitchy-paint-49930405
#include <QtWidgets>
#include <array>

class Render3dModel : public QWidget {
   struct Model {
      mutable int counter = {};
      QSize size;
      QFont font{"Helvetica", 48};
      QImage getImage() const {
         static auto const format = QPixmap{1,1}.toImage().format();
         QImage image{size, format};
         image.fill((counter & 1) ? Qt::blue : Qt::yellow);
         QPainter p(&image);
         p.setFont(font);
         p.setPen((counter & 1) ? Qt::yellow : Qt::blue);
         p.drawText(image.rect(), QString::number(counter));
         counter++;
         return image;
      }
   } m_model;
   void paintEvent(QPaintEvent *) override {
      m_model.size = size();
      auto image = m_model.getImage();
      QPainter{this}.drawImage(QPoint{0, 0}, image);
   }
};

int main(int argc, char **argv) {
   QApplication app{argc, argv};
   QWidget win;
   QVBoxLayout topLayout{&win};
   QTabWidget tabs;
   topLayout.addWidget(&tabs);
   // Tabs
   for (auto text : { "Shape", "Dimensions", "Layout"}) tabs.addTab(new QWidget, text);
   tabs.setCurrentIndex(1);
   QHBoxLayout tabLayout{tabs.currentWidget()};
   QGroupBox dims{"Section Dimensions"}, model{"3D Model"};
   QGridLayout dimsLayout{&dims}, modelLayout{&model};
   for (auto w : {&dims, &model}) tabLayout.addWidget(w);
   // Section Dimensions
   for (auto text : {"Diameter 1", "Diameter 2", "Length"}) {
      auto row = dimsLayout.rowCount();
      std::array<QWidget*, 3> widgets{{new QLabel{text}, new QDoubleSpinBox, new QLabel{"inch"}}};
      for (auto *w : widgets)
         dimsLayout.addWidget(w, row, dimsLayout.count() % widgets.size());
   }
   tabLayout.setAlignment(&dims, Qt::AlignLeft | Qt::AlignTop);
   dims.setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
   // Model
   Render3dModel render;
   modelLayout.addWidget(&render);
   model.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
   model.setMinimumSize(250, 250);
   win.show();
   return app.exec();
}
