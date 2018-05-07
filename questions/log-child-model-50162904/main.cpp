// https://github.com/KubaO/stackoverflown/tree/master/questions/log-child-model-50162904
#include <QtWidgets>

// --see https://stackoverflow.com/a/21653558/1329652
namespace detail {
template <typename F>
struct FEvent : QEvent {
   using Fun = typename std::decay<F>::type;
   const QObject *const obj;
   const QMetaObject *const type = obj->metaObject();
   Fun fun;
   template <typename Fun>
   FEvent(const QObject *obj, Fun &&fun) : QEvent(QEvent::None), obj(obj), fun(std::forward<Fun>(fun)) {}
   ~FEvent() {
      if (obj->metaObject()->inherits(type)) // ensure that the object is not being destructed
         fun();
   }
}; }

template <typename F>
static void postToObject(F &&fun, QObject *obj = qApp) {
   if (qobject_cast<QThread*>(obj))
      qWarning() << "posting a call to a thread object - consider using postToThread";
   QCoreApplication::postEvent(obj, new detail::FEvent<F>(obj, std::forward<F>(fun)));
}
// --end

class LogViewAdapter : public QObject {
   bool viewAtBottom = true;
   static void insert(QAbstractItemModel *model, const QString &msg) {
      auto row = model->rowCount();
      model->insertRows(row, 1);
      model->setData(model->index(row, 0), msg);
   }
public:
   QAbstractItemView *view() const { return static_cast<QAbstractItemView*>(parent()); }
   LogViewAdapter(QAbstractItemView *view) : QObject(view) {
      auto *model = view->model();
      connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this, [=]{
         auto bar = this->view()->verticalScrollBar();
         viewAtBottom = bar ? (bar->value() == bar->maximum()) : false;
      });
      connect(model, &QAbstractItemModel::rowsInserted, this, [=]{
         if (viewAtBottom) this->view()->scrollToBottom();
      });
   }
   /// This method is thread-safe
   void send(const QString &msg) {
      if (QThread::currentThread() == thread())
         insert(view()->model(), msg);
      else
         postToObject([this, msg]{ insert(view()->model(), msg); }, view()->model());
   }
};

int main(int argc, char *argv[]) {
   QApplication app{argc, argv};

   QPushButton parent{"Click to close"};
   QStringListModel logModel;
   QListView logView{&parent};
   logView.setModel(&logModel);
   logView.setWindowFlag(Qt::Window); // <---
   logView.setUniformItemSizes(true);
   LogViewAdapter adapter{&logView};

   QTimer timer;
   timer.start(100);
   QObject::connect(&timer, &QTimer::timeout, []{ qDebug() << QDateTime::currentDateTimeUtc(); });
   QObject::connect(&parent, &QPushButton::clicked, &parent, &QWidget::close);
   parent.setMinimumSize(240, 64);
   logView.setMinimumSize(640, 480);
   parent.show();
   logView.move(parent.frameGeometry().topRight());
   logView.show();
   static QPointer<LogViewAdapter> staticAdapter{&adapter};
   qInstallMessageHandler([](auto, auto &, auto &msg){
      if (staticAdapter) staticAdapter->send(msg);
   });
   return app.exec();
}
