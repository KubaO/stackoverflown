#include <QtWidgets>

const auto mimeType = QStringLiteral("application/x-qabstractitemmodeldatalist");

class SpriteModel : public QAbstractListModel {
public:
   typedef QSharedPointer<QOpenGLTexture> TexturePtr;
   typedef QPair<QImage, TexturePtr> Item;
private:
   QList<Item> m_imageList;
public:
   SpriteModel(QObject * parent = 0) : QAbstractListModel(parent) {}

   static Item makeItem(const QImage & image) {
      return qMakePair(image, TexturePtr(new QOpenGLTexture(image)));
   }

   void setContents(QList<Item> &newList) {
      beginResetModel();
      m_imageList = newList;
      endResetModel();
   }

   int rowCount(const QModelIndex &) const Q_DECL_OVERRIDE {
      return m_imageList.size();
   }

   QVariant data(const QModelIndex & index, int role) const Q_DECL_OVERRIDE {
      if (role == Qt::DecorationRole)
         return m_imageList[index.row()].first;
      else if (role == Qt::DisplayRole)
         return "";
      else
         return QVariant();
   }

   Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE {
      return Qt::CopyAction | Qt::MoveAction;
   }

   Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE {
      auto defaultFlags = QAbstractListModel::flags(index);
      if (index.isValid())
         return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
      else
         return Qt::ItemIsDropEnabled | defaultFlags;
   }

   bool removeRows(int position, int rows, const QModelIndex &) Q_DECL_OVERRIDE {
      beginRemoveRows(QModelIndex(), position, position+rows-1);
      for (int row = 0; row < rows; ++row) {
         m_imageList.removeAt(position);
      }
      endRemoveRows();
      return true;
   }

   bool insertRows(int position, int rows, const QModelIndex &) Q_DECL_OVERRIDE {
      beginInsertRows(QModelIndex(), position, position+rows-1);
      auto size = m_imageList.isEmpty() ? QSize(10, 10) : m_imageList.at(0).first.size();
      QImage img(size, m_imageList[0].first.format());
      for (int row = 0; row < rows; ++row) {
         m_imageList.insert(position, makeItem(img));
      }
      endInsertRows();
      return true;
   }

   bool setData(const QModelIndex &index, const QVariant &value, int role) Q_DECL_OVERRIDE {
      if (index.isValid() && role == Qt::EditRole) {
         m_imageList.replace(index.row(), makeItem(value.value<QImage>()));
         emit dataChanged(index, index);
         return true;
      }
      return false;
   }

   QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE {
      auto mimeData = new QMimeData();
      QByteArray encodedData;
      QDataStream stream(&encodedData, QIODevice::WriteOnly);

      qDebug() << "mimeData" << indexes;

      for (const auto & index : indexes) {
         if (! index.isValid()) continue;
         QMap<int, QVariant> roleDataMap;
         roleDataMap[Qt::DecorationRole] = data(index, Qt::DecorationRole);
         stream << index.row() << index.column() << roleDataMap;
      }
      mimeData->setData(mimeType, encodedData);
      return mimeData;
   }

   bool canDropMimeData(const QMimeData *data,
                        Qt::DropAction, int, int column, const QModelIndex & parent) const Q_DECL_OVERRIDE
   {
      return data->hasFormat(mimeType) && (column == 0 || (column == -1 && parent.column() == 0));
   }

   bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) Q_DECL_OVERRIDE {
      Q_UNUSED(column);
      qDebug() << "drop" << action << row << column << parent;
      if (! data->hasFormat(mimeType)) return false;

      auto encoded = data->data(mimeType);
      QDataStream stream(&encoded, QIODevice::ReadOnly);
      QList<QImage> images;
      while (! stream.atEnd()) {
         int row, col;
         QMap<int, QVariant> roleDataMap;
         stream >> row >> col >> roleDataMap;
         auto it = roleDataMap.find(Qt::DecorationRole);
         if (it != roleDataMap.end()) {
            images << it.value().value<QImage>();
         }
      }
      if (row == -1) row = parent.row();
      if (! images.isEmpty()) {
         beginInsertRows(parent, row, row+images.size() - 1);
         qDebug() << "inserting" << images.count();
         for (auto & image : images)
            m_imageList.insert(row ++, makeItem(image));
         endInsertRows();
         return true;
      }
      return false;
   }
};

QImage makeImage(int n) {
   QImage img(64, 128, QImage::Format_RGBA8888);
   img.fill(Qt::transparent);
   QPainter p(&img);
   p.setFont(QFont("Helvetica", 32));
   p.drawText(img.rect(), Qt::AlignCenter, QString::number(n));
   return img;
}

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QList<SpriteModel::Item> items;
   for (int i = 0; i < 5; ++i) items << SpriteModel::makeItem(makeImage(i));
   SpriteModel model;
   model.setContents(items);
   QListView view;
   view.setModel(&model);
   view.setViewMode(QListView::IconMode);
   view.setSelectionMode(QAbstractItemView::ExtendedSelection);
   view.setDragEnabled(true);
   view.setAcceptDrops(true);
   view.setDropIndicatorShown(true);
   view.show();
   return a.exec();
}
