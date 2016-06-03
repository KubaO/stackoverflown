// https://github.com/KubaO/stackoverflown/tree/master/questions/lambda-list-37615204
#include <QtWidgets>

class Widget : public QWidget {
   struct Buyable {
      void buy(int) const {}
   };
   QList<Buyable> m_list;
   QList<QList<QWidget*>> m_widgets;
   int m_amountMultiplier = 2;
public:
   Widget() {
      auto button = [this]{ return new QPushButton(this); };
      for (int i = 0; i < 5; ++i) {
         m_list << Buyable();
         m_widgets << QList<QWidget*>{button(), button(), button()};
      }
   }
   void test() {
      Q_ASSERT(m_list.size() == m_widgets.size());
      for (int i = 0; i < m_list.size(); ++i) {
         connect(static_cast<QAbstractButton*>(m_widgets.at(i).at(2)),
                 &QAbstractButton::clicked,
                 [this, i]() {
            m_list.at(i).buy(m_amountMultiplier);}
         );
      }
   }
};

int main() {}

