// https://github.com/KubaO/stackoverflown/tree/master/questions/connect-bind-44081724
#include <QtWidgets>
#include <functional>

class Class : public QWidget
{
   Q_OBJECT
   QPushButton button;
   struct CTL {
      void saveas(const QString &);
   } ctl;
   struct Workflow_Table {
       void addRow() {}
   } workflow_table;

public:
   Class() {
      connect(&button, &QPushButton::clicked, this, std::bind(&CTL::saveas, &ctl, "saved.ctl"));
      connect(&button, &QPushButton::clicked, this, [this]{ workflow_table.addRow(); });
      connect(&button, &QPushButton::clicked, this, std::bind(&Workflow_Table::addRow, &workflow_table));

      // better yet
      connect(&button, &QPushButton::clicked, this, [this]{ ctl.saveas("saved.ctl"); });
      connect(&button, &QPushButton::clicked, this, [this]{ workflow_table.addRow(); });

      // or simply, if we know that `this` will outlive `button` (they do by design here),
      // and that both button and this live in the same thread (again, they do by design here)
      connect(&button, &QPushButton::clicked, [this]{ ctl.saveas("saved.ctl"); });
      connect(&button, &QPushButton::clicked, [this]{ workflow_table.addRow(); });
   }
};

int main() {}
#include "main.moc"
