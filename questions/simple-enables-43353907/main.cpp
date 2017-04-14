// https://github.com/KubaO/stackoverflown/tree/master/questions/simple-enables-43353907
#include <QtWidgets>
#include <array>

class Base : public QWidget {
   QVBoxLayout layout{this};
   QCheckBox enabled{"Enabled"}; // must be declared after the layout
   QGroupBox group{"Group"};
   QHBoxLayout groupLayout{&group};
   QLineEdit edit1{"1"}; // must be declared after the group box
   QLineEdit edit2{"2"};
   const std::array<QWidget*, 2> groupWidgets = {&edit1, &edit2};

   const std::array<void(Base::*)(bool), 4> methods{
      &Base::setEnabledLayout, &Base::setEnabledGroup,
            &Base::setEnabledChildren, &Base::setEnabledArray
   };
   decltype(methods)::const_iterator method = methods.begin();
   void setEnabledLayout(bool enabled) {
      for (int i = 0; i < groupLayout.count(); i++)
         if (auto widget = groupLayout.itemAt(i)->widget())
            widget->setEnabled(enabled);
   }
   void setEnabledGroup(bool enabled) {
      group.setEnabled(enabled);
   }
   void setEnabledChildren(bool enabled) {
      for (auto child : group.findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
           child->setEnabled(enabled);
   }
   void setEnabledArray(bool enabled) {
      for (auto widget : groupWidgets)
         widget->setEnabled(enabled);
   }
public:
   explicit Base(QWidget* parent = {}) : QWidget{parent} {
      layout.addWidget(&enabled);
      layout.addWidget(&group);
      groupLayout.addWidget(&edit1);
      groupLayout.addWidget(&edit2);
      connect(&enabled, &QCheckBox::toggled, [this](bool enabled){
         (this->**method)(enabled); // see http://stackoverflow.com/q/12189057/1329652
         // switch methods only when in enabled state
         if (enabled)
            if (++method == methods.end()) method = methods.begin();
      });
      enabled.setChecked(true);
   }
};

int main(int argc, char* argv[]) {
   QApplication a{argc, argv};
   Base w;
   w.show();
   return a.exec();
}
