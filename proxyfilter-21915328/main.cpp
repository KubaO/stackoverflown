#include <QApplication>
#include <QSortFilterProxyModel>
#include <QListView>
#include <QStandardItemModel>

class Foo {};

class QSortFilterProxyModel_NumbersLast : public QSortFilterProxyModel
{
   Q_OBJECT
public:
   QSortFilterProxyModel_NumbersLast(QObject * parent = nullptr) :
      QSortFilterProxyModel(parent) {}
   bool lessThan(const QModelIndex &, const QModelIndex &) const {
      return false;
   }
   Q_SIGNAL void dummySignal(Foo);
   void dummy() {
      QObject::connect(this, &QSortFilterProxyModel_NumbersLast::dummySignal, [](Foo){});
   }
};

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   QStandardItemModel * model = new QStandardItemModel(&app);
   QSortFilterProxyModel_NumbersLast *proxyModel = new QSortFilterProxyModel_NumbersLast(&app);
   proxyModel->setSourceModel(model);
   QListView view;
   view.setModel(model);
   view.show();
   model->appendRow(new QStandardItem("Foo"));
   model->appendRow(new QStandardItem("Bar"));
   model->appendRow(new QStandardItem("Baz"));
   return app.exec();
}

#include "main.moc"
