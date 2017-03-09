// https://github.com/KubaO/stackoverflown/tree/master/questions/tempconvert-42648860
#include <QtWidgets>

const char kUnit[] = "unit";
class MyWidget : public QWidget {
   QFormLayout m_layout{this};
   QDoubleSpinBox m_fahrenheit, m_celsius, m_kelvin;
   QList<QDoubleSpinBox*> const m_spinBoxes{&m_fahrenheit, &m_celsius, &m_kelvin};
   enum Unit { Fahrenheit, Celsius, Kelvin };

   static double Q_DECL_RELAXED_CONSTEXPR fromK(Unit to, double val) {
      if (to == Fahrenheit) return (val-273.15)*1.8 + 32.0;
      else if (to == Celsius) return val - 273.15;
      else return val;
   }
   static double Q_DECL_RELAXED_CONSTEXPR toK(Unit from, double val) {
      if (from == Fahrenheit) return (val-32.0)/1.8 + 273.15;
      else if (from == Celsius) return val + 273.15;
      else return val;
   }
   void setTemp(Unit unit, double temp,  QDoubleSpinBox * skip = nullptr) {
      for (auto spin : m_spinBoxes) if (spin != skip) {
         QSignalBlocker b{spin};
         spin->setValue(fromK(unitOf(spin), toK(unit, temp)));
      }
   }
   static Unit unitOf(QDoubleSpinBox * spin) {
      return static_cast<Unit>(spin->property(kUnit).toInt());
   }
public:
   MyWidget(QWidget * parent = nullptr) : QWidget{parent} {
      m_layout.addRow("Fahreneheit", &m_fahrenheit);
      m_layout.addRow("Celsius", &m_celsius);
      m_layout.addRow("Kelvin", &m_kelvin);
      m_fahrenheit.setProperty(kUnit, Fahrenheit);
      m_celsius.setProperty(kUnit, Celsius);
      m_kelvin.setProperty(kUnit, Kelvin);
      for (auto const spin : m_spinBoxes) {
         auto const unit = unitOf(spin);
         spin->setRange(fromK(unit, 0.), fromK(unit, 1000.));
         connect(spin, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                 [=]{ setTemp(unit, spin->value(), spin); });
      }
      setTemp(Celsius, 20.);
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   MyWidget ui;
   ui.show();
   return app.exec();
}
