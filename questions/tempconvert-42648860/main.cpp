// https://github.com/KubaO/stackoverflown/tree/master/questions/tempconvert-42648860
#include <QtWidgets>

class MyWidget : public QWidget {
   QFormLayout m_layout{this};
   QDoubleSpinBox m_fahrenheit, m_celsius, m_kelvin;
   enum Unit { Fahrenheit, Celsius, Kelvin };
   static double Q_DECL_RELAXED_CONSTEXPR fromK(Unit to, double val) {
      if (to == Fahrenheit) return (val-273.15)*1.8 + 32.0;
      else if (to == Celsius) return val - 273.15;
      else return val;
   }
   static double Q_DECL_RELAXED_CONSTEXPR toK(Unit from, double val) {
      if (from == Fahrenheit) return (val-32.0)/1.8+273.15;
      else if (from == Celsius) return val + 273.15;
      else return val;
   }
   void setTemp(double kelvin) {
      QSignalBlocker b1{&m_fahrenheit}, b2{&m_celsius}, b3{&m_kelvin};
      m_fahrenheit.setValue(fromK(Fahrenheit, kelvin));
      m_celsius.setValue(fromK(Celsius, kelvin));
      m_kelvin.setValue(kelvin);
   }
public:
   MyWidget(QWidget * parent = nullptr) : QWidget{parent} {
      m_layout.addRow("Fahreneheit", &m_fahrenheit);
      m_layout.addRow("Celsius", &m_celsius);
      m_layout.addRow("Kelvin", &m_kelvin);
      auto const maxKelvins = 1000.;
      m_fahrenheit.setRange(fromK(Fahrenheit, 0.), fromK(Fahrenheit, maxKelvins));
      m_celsius.setRange(fromK(Celsius, 0.), fromK(Celsius, maxKelvins));
      m_kelvin.setRange(0., maxKelvins);
      auto const signal =
            static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged);
      connect(&m_fahrenheit, signal, [this]{ setTemp(toK(Fahrenheit, m_fahrenheit.value())); });
      connect(&m_celsius, signal, [this]{ setTemp(toK(Celsius, m_celsius.value())); });
      connect(&m_kelvin, signal, [this]{ setTemp(toK(Kelvin, m_kelvin.value())); });
      setTemp(toK(Celsius, 20.));
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   MyWidget ui;
   ui.show();
   return app.exec();
}
