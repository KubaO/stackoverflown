#if 1

// https://github.com/KubaO/stackoverflown/tree/master/questions/scrollarea-filter-40605540
#include <QtWidgets>

class Tracker : public QFrame {
   QPoint pos;
   void invalidatePos() { pos.setX(-1); }
   bool isPosInvalid() const { return pos.x() < 0; }
   void mouseMoveEvent(QMouseEvent *event) override {
      pos = event->pos();
      update();
   }
   void paintEvent(QPaintEvent *event) override {
      QFrame::paintEvent(event);
      if (isPosInvalid()) return;
      QPainter p{this};
      p.setPen(Qt::red);
      p.setBrush(Qt::red);
      p.drawEllipse(pos, 4, 4);
   }
   void leaveEvent(QEvent *event) {
      invalidatePos();
      update();
      QFrame::leaveEvent(event);
   }
public:
   Tracker(QWidget * parent = nullptr) : QFrame{parent} {
      setFrameStyle(QFrame::Panel);
      setLineWidth(2);
      setMouseTracking(true);
   }
};

class TopWidget : public QWidget {
   QVBoxLayout m_layout{this};
   QScrollArea m_area;
   QWidget m_child;
   Tracker m_tracker{&m_child};
public:
   TopWidget(QWidget * parent = nullptr) : QWidget{parent} {
      m_layout.addWidget(&m_area);
      m_area.setWidget(&m_child);
      m_child.setMinimumSize(1024, 1024);
      m_tracker.setGeometry(150, 150, 300, 300);
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   TopWidget ui;
   ui.show();
   return app.exec();
}

#else

#include <QtWidgets>

class CoordinateDialog : public QDialog {
   Q_OBJECT
   Q_PROPERTY(QVector3D value READ value WRITE setValue NOTIFY coordinatesChanged)
   QVector3D m_value;
   QFormLayout m_layout{this};
   QDoubleSpinBox m_x, m_y, m_z;
   QDialogButtonBox m_buttons;
public:
   CoordinateDialog(QWidget *parent = nullptr) : CoordinateDialog(QVector3D(), parent) {}
   CoordinateDialog(const QVector3D &value, QWidget *parent = nullptr) :
      QDialog{parent}, m_value(value)
   {
      m_layout.addRow("X", &m_x);
      m_layout.addRow("Y", &m_y);
      m_layout.addRow("Z", &m_z);
      m_layout.addRow(&m_buttons);
      m_buttons.addButton(QDialogButtonBox::Ok);
      m_buttons.addButton(QDialogButtonBox::Cancel);
      connect(&m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
      connect(&m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
      connect(&m_x, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
              [=](double x){ auto v = m_value; v.setX(x); setValue(v); });
      connect(&m_y, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
              [=](double y){ auto v = m_value; v.setY(y); setValue(v); });
      connect(&m_z, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
              [=](double z){ auto v = m_value; v.setZ(z); setValue(v); });
   }
   Q_SIGNAL void coordinatesChanged(const QVector3D &);
   Q_SIGNAL void coordinatesAccepted(const QVector3D &);
   void accept() override {
      emit coordinatesAccepted(m_value);
      QDialog::accept();
   }
   QVector3D value() const { return m_value; }
   Q_SLOT void setValue(const QVector3D &value) {
      if (m_value == value) return;
      m_value = value;
      m_x.setValue(m_value.x());
      m_y.setValue(m_value.y());
      m_z.setValue(m_value.z());
      emit coordinatesChanged(m_value);
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   CoordinateDialog ui;
   ui.show();
   return app.exec();
}

#include "main.moc"

#endif
