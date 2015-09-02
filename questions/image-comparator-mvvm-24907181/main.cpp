#include <QApplication>
#include <QTableView>
#include <QIdentityProxyModel>
#include <QStandardItemModel>
#include <QPainter>

/** Adds image comparison results to a table/tree with pairs of images in each row.
 *
 * This is a viewmodel that expects a table or tree with row containing pairs of images in the
 * first two columns. Comparison results are visualized as the added last column.
 * A null result is provided for rows that don't have two images as their first two columns.
 */
class Comparator : public QIdentityProxyModel {
   Q_OBJECT
   bool isLastColumn(const QModelIndex & proxyIndex) const {
      return proxyIndex.column() == columnCount(proxyIndex.parent()) - 1;
   }
   QModelIndex indexInColumn(int column, const QModelIndex & proxyIndex) const {
      return index(proxyIndex.row(), column, proxyIndex.parent());
   }
   /** Compares the two images, returning their difference..
    * Both images are expanded to the larger of their sizes. Missing data is filled with
    * transparent pixels. The images can be in any format. The difference is in ARGB32. */
   QImage compare(const QImage & left, const QImage & right) const {
      QImage delta(left.size().expandedTo(right.size()), QImage::Format_ARGB32);
      delta.fill(Qt::transparent);
      QPainter p(&delta);
      p.setRenderHint(QPainter::Antialiasing);
      p.drawImage(0, 0, left);
      p.setCompositionMode(QPainter::CompositionMode_Difference);
      p.drawImage(0, 0, right);
      return delta;
   }
public:
   Comparator(QObject * parent = 0) : QIdentityProxyModel(parent) {}
   QModelIndex index(int row, int column, const QModelIndex &parent) const Q_DECL_OVERRIDE {
      if (column != columnCount(parent) - 1)
         return QIdentityProxyModel::index(row, column, parent);
      return createIndex(row, column, parent.internalPointer());
   }
   int columnCount(const QModelIndex &parent) const Q_DECL_OVERRIDE {
      return sourceModel()->columnCount(mapToSource(parent)) + 1;
   }
   QVariant data(const QModelIndex &proxyIndex, int role) const Q_DECL_OVERRIDE {
      if (isLastColumn(proxyIndex)) {
         QVariant left = data(indexInColumn(0, proxyIndex), role);
         QVariant right = data(indexInColumn(1, proxyIndex), role);
         if (!left.canConvert<QImage>() || !right.canConvert<QImage>()) return QVariant();
         return QVariant::fromValue(compare(left.value<QImage>(), right.value<QImage>()));
      }
      return QAbstractProxyModel::data(proxyIndex, role);
   }
};

QImage sector(qreal diameter, qreal size, qreal start, qreal end, const QColor & color)
{
   QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
   image.fill(Qt::transparent);
   QPainter p(&image);
   p.setRenderHint(QPainter::Antialiasing);
   p.setPen(Qt::NoPen);
   p.setBrush(color);
   p.drawPie(QRectF(size-diameter, size-diameter, diameter, diameter),
             qRound(start*16), qRound((end-start)*16));
   return image;
}

QStandardItem * imageItem(const QImage & image) {
   QScopedPointer<QStandardItem> item(new QStandardItem);
   item->setEditable(false);
   item->setSelectable(false);
   item->setData(QVariant::fromValue(image), Qt::DecorationRole);
   item->setSizeHint(image.size());
   return item.take();
}

typedef QList<QStandardItem*> QStandardItemList;

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QStandardItemModel images;
   Comparator comparator;
   QTableView view;
   comparator.setSourceModel(&images);
   view.setModel(&comparator);

   images.appendRow(QStandardItemList()
                    << imageItem(sector(150, 160, 30, 100, Qt::red))
                    << imageItem(sector(150, 160, 60, 120, Qt::blue)));
   images.appendRow(QStandardItemList()
                    << imageItem(sector(40, 45, 0, 180, Qt::darkCyan))
                    << imageItem(sector(40, 45, 180, 360, Qt::cyan)));

   view.resizeColumnsToContents();
   view.resizeRowsToContents();
   view.adjustSize();
   view.show();

   return a.exec();
}

#include "main.moc"
