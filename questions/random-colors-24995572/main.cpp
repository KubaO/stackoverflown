#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QGridLayout>
#include <QPainter>
#include <random>

std::default_random_engine rng;

class Dialog : public QWidget {
   Q_OBJECT
   Q_PROPERTY(bool recolorOnResize READ recolorOnResize WRITE setRecolorOnResize)
   QList<QColor> m_palette;
   QList<QList<QColor>> m_chosenColors;
   bool m_recolorOnResize;
   void paintEvent(QPaintEvent *) {
      QPainter p(this);
      p.fillRect(rect(), Qt::white);
      p.setRenderHint(QPainter::Antialiasing);

      int rec_size=64;
      int rows=height()/rec_size;
      int cols=width()/rec_size;
      std::uniform_int_distribution<int> dist(0, m_palette.size()-1);
      while (m_chosenColors.size() < rows) m_chosenColors << QList<QColor>();
      for (QList<QColor> & colors : m_chosenColors)
         while (colors.size() < cols)
            colors << m_palette.at(dist(rng));

      QPointF points[4];
      for (int i=0; i<rows; i++) {
         for (int j=0; j<cols; j++) {
            points[0] = QPointF(rec_size*(j),rec_size*(i+0.5));
            points[1] = QPointF(rec_size*(j+0.5),rec_size*(i));
            points[2] = QPointF(rec_size*(j+1),rec_size*(i+0.5));
            points[3] = QPointF(rec_size*(j+0.5),rec_size*(i+1));
            p.setBrush(m_chosenColors[i][j]);
            p.drawPolygon(points, 4);
         }
      }
   }
   void resizeEvent(QResizeEvent *) {
      if (m_recolorOnResize) m_chosenColors.clear();
   }
public:
   Dialog(QWidget * parent = 0) : QWidget(parent), m_recolorOnResize(false) {
      m_palette << "#E2C42D" << "#E5D796" << "#BEDA2C" << "#D1DD91" << "#E2992D" << "#E5C596";
      setAttribute(Qt::WA_OpaquePaintEvent);
   }
   Q_SLOT void randomize() {
      m_chosenColors.clear();
      update();
   }
   bool recolorOnResize() const { return m_recolorOnResize; }
   void setRecolorOnResize(bool recolor) {
      m_recolorOnResize = recolor;
      setAttribute(Qt::WA_StaticContents, !m_recolorOnResize);
   }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QWidget w;
   QGridLayout l(&w);
   Dialog d;
   QCheckBox recolor("Recolor on Resize");
   QPushButton update("Repaint"), randomize("Randomize");
   d.setMinimumSize(256, 128);
   l.addWidget(&d, 0, 0, 1, 2);
   l.addWidget(&recolor, 1, 0, 1, 2);
   l.addWidget(&update, 2, 0);
   l.addWidget(&randomize, 2, 1);
   recolor.setChecked(d.recolorOnResize());
   QObject::connect(&recolor, &QAbstractButton::toggled, [&d](bool checked){
      d.setRecolorOnResize(checked);}
   );
   QObject::connect(&update, &QAbstractButton::clicked, &d, static_cast<void(QWidget::*)()>(&QWidget::update));
   QObject::connect(&randomize, &QAbstractButton::clicked, &d, &Dialog::randomize);
   w.show();
   return a.exec();
}

#include "main.moc"
