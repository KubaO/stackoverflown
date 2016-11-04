// https://github.com/KubaO/stackoverflown/tree/master/questions/list-widgets-40403640
#include <QtWidgets>

class StandardItemModel : public QStandardItemModel {
   bool moveRows(const QModelIndex &srcParent, int srcRow, int count,
                 const QModelIndex &dstParent, int dstRow) override {
      if (count == 0) return true;
      if (count != 1 || srcParent != dstParent) return false;
      if (srcRow == dstRow) return true;
      if (abs(srcRow - dstRow) != 1) return false;
      auto root = srcParent.isValid() ? itemFromIndex(srcParent) : invisibleRootItem();
      if (!root) return false;
      auto srcItem = root->takeChild(srcRow);
      auto dstItem = root->takeChild(dstRow);
      if (!srcItem || !dstItem) return false;
      root->setChild(srcRow, dstItem);
      root->setChild(dstRow, srcItem);
      return true;
   }
public:
   using QStandardItemModel::QStandardItemModel;
};

class ListUi : public QWidget {
   Q_OBJECT
   QGridLayout m_layout{this};
   QVBoxLayout m_column;
   QPushButton m_up{"⬆"}, m_down{"⬇"}, m_remove{"−"}, m_add{"+"};
   QLabel m_caption;
   QListView m_list;
   inline QAbstractItemModel * model() const { return m_list.model(); }
   inline QModelIndex root() const { return m_list.rootIndex(); }
   inline QModelIndex index(int row) const { return model()->index(row, 0, root()); }
public:
   ListUi(const QString & caption, QWidget * parent = nullptr) :
      QWidget{parent},
      m_caption{caption}
   {
      m_layout.addWidget(&m_up, 0, 0);
      m_layout.addWidget(&m_down, 1, 0, 1, 1, Qt::AlignTop);
      m_layout.addLayout(&m_column, 0, 1, 3, 1);
      m_column.addWidget(&m_caption);
      m_column.addWidget(&m_list);
      m_layout.addWidget(&m_remove, 0, 2);
      m_layout.addWidget(&m_add, 2, 2);
      m_caption.setAlignment(Qt::AlignCenter);
      connect(&m_add, &QPushButton::clicked, [this]{
         int row = model()->rowCount(root());
         if (model()->columnCount(root()) == 0)
            model()->insertColumn(0, root());
         if (model()->insertRow(row, root())) {
            m_list.setCurrentIndex(index(row));
            m_list.edit(index(row));
         }
      });
      connect(&m_remove, &QPushButton::clicked, [this]{
         if (m_list.currentIndex().isValid())
            model()->removeRow(m_list.currentIndex().row(), root());
      });
      connect(&m_up, &QPushButton::clicked, [this]{
         auto row = m_list.currentIndex().row();
         if (row > 0 && model()->moveRow(root(), row, root(), row - 1))
            m_list.setCurrentIndex(index(row-1));
      });
      connect(&m_down, &QPushButton::clicked, [this]{
         auto row = m_list.currentIndex().row();
         if (row >= 0 && row < (model()->rowCount(root()) - 1) &&
             model()->moveRow(root(), row, root(), row + 1))
            m_list.setCurrentIndex(index(row+1));
      });
   }
   void setModel(QAbstractItemModel * model) {
      m_list.setModel(model);
      connect(m_list.selectionModel(), &QItemSelectionModel::currentChanged, this, &ListUi::currentIndexChanged);
   }
   void setRootIndex(const QModelIndex & index) {
      m_list.setRootIndex(index);
   }
   Q_SIGNAL void currentIndexChanged(const QModelIndex &);
};

class Window : public QWidget {
   QGridLayout m_layout{this};
   ListUi m_programs{"Liste des programmes"};
   ListUi m_sessions{"Liste des sessions"};
   StandardItemModel m_model;
public:
   explicit Window(QWidget * parent = nullptr) : QWidget{parent}
   {
      m_layout.addWidget(&m_programs, 0, 0);
      m_layout.addWidget(&m_sessions, 0, 1);
      m_programs.setModel(&m_model);
      m_sessions.setModel(&m_model);
      m_sessions.setDisabled(true);
      connect(&m_programs, &ListUi::currentIndexChanged, [this](const QModelIndex & root){
         m_sessions.setEnabled(true);
         m_sessions.setRootIndex(root);
      });
   }
   void setJson(const QJsonDocument & doc) {
      m_model.clear();
      auto object = doc.object();
      for (auto it = object.begin(); it != object.end(); ++it) {
         if (!m_model.columnCount()) m_model.appendColumn({});
         auto root = new QStandardItem(it.key());
         m_model.appendRow(root);
         if (it.value().isArray()) {
            auto array = it.value().toArray();
            for (auto const & value : array) {
               if (!root->columnCount()) root->appendColumn({});
               root->appendRow(new QStandardItem{value.toString()});
            }
         }
      }
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   auto json = R"--({
    "Foo":["Foo-1", "Foo-2", "Foo-3"],
    "Bar":["Bar-1", "Bar-2"]
    })--";
   Window ui;
   ui.setJson(QJsonDocument::fromJson(json));
   ui.show();
   return app.exec();
}

#include "main.moc"
