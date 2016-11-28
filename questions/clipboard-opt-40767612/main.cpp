// https://github.com/KubaO/stackoverflown/tree/master/questions/clipboard-opt-40767612
#include <QtWidgets>
#include <QtConcurrent>

class Data {
   Q_DISABLE_COPY(Data) // presumably expensive to copy
public:
   //...
   QMimeData* serialize() { return new QMimeData; }
   static QSharedPointer<Data> deserialize(QMimeData*);
};
Q_DECLARE_METATYPE(QSharedPointer<Data>)

QMimeData* clone(const QMimeData *src) {
   auto dst = new QMimeData();
   for (auto format : src->formats())
      dst->setData(format, src->data(format));
   return dst;
}

class Class : public QObject {
   Q_OBJECT
   QSharedPointer<Data> m_data;
   enum { None, Copied, Ready } m_internalCopy = None;

   Q_SIGNAL void reqPaste(const QSharedPointer<Data> &);
   void paste(const QSharedPointer<Data> &);
   void onDataChanged() {
      m_internalCopy = m_internalCopy == Copied ? Ready : None;
   }
public:
   Q_SLOT void on_copy() {
      m_internalCopy = Copied;
      QScopedPointer<QMimeData> mimeData(m_data->serialize());
      QApplication::clipboard()->setMimeData(mimeData.data());
   }
   Q_SLOT void on_paste() {
      if (m_internalCopy == Ready)
         return paste(m_data);

      m_internalCopy = None;
      auto mimeData = clone(QApplication::clipboard()->mimeData());
      QtConcurrent::run([=]{
         emit reqPaste(Data::deserialize(mimeData));
         delete mimeData;
      });
   }
   Class() {
      qRegisterMetaType<QSharedPointer<Data>>();
      connect(QApplication::clipboard(), &QClipboard::dataChanged, this,
              &Class::onDataChanged);
      connect(this, &Class::reqPaste, this, &Class::paste);
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   return app.exec();
}
