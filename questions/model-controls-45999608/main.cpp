// https://github.com/KubaO/stackoverflown/tree/master/questions/model-controls-45999608
#include <QtWidgets>

class SpinboxDelegate : public QStyledItemDelegate {};

class Ui : public QWidget {
   Q_OBJECT
   QVBoxLayout m_layout{this};
   QTableView m_view;
   QDialogButtonBox m_box;
   QPushButton m_add{"Add"}, m_remove{"Remove"};
   SpinboxDelegate m_spinboxDelegate;
public:
   Ui(QWidget * parent = {}) : QWidget(parent) {
      m_layout.addWidget(&m_view);
      m_layout.addWidget(&m_box);
      m_box.addButton(&m_add, QDialogButtonBox::ActionRole);
      m_box.addButton(&m_remove, QDialogButtonBox::ActionRole);
      connect(&m_add, &QPushButton::clicked, this, &Ui::add);
      connect(&m_remove, &QPushButton::clicked, this, &Ui::remove);
      m_view.setItemDelegateForColumn(0, &m_spinboxDelegate);
   }
   void setModel(QAbstractItemModel * model) {
      m_view.setModel(model);
   }
   Q_SIGNAL void add();
   Q_SIGNAL void remove();
};

int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   Ui ui;
   ui.show();
   QStandardItemModel model;
   ui.setModel(&model);

   return app.exec();
}

#include "main.moc"
