// https://github.com/KubaO/stackoverflown/tree/master/questions/lambda-list-37615204
#include <QtWidgets>

struct Building {
   void buy() {}
};

class Class : public QObject {
   QList<Building> m_buildings;
   QList<QList<QWidget*>> m_widgets;
public:
   Class() {
      for (int i = 0; i<m_buildings.size(); ++i)
         connect(static_cast<QAbstractButton*>(m_widgets.at(i).at(2)), &QAbstractButton::clicked, [=] {
            m_buildings[i].buy();
         });
   }
};

int main() {}
