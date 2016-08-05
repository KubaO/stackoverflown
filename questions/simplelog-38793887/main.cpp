// https://github.com/KubaO/stackoverflown/tree/master/questions/simplelog-38793887
#include <QtWidgets>
#include <QtConcurrent>
#include "Log.h"

int main(int argc, char ** argv) {
   using Q = QObject;
   QApplication app{argc, argv};
   QStringListModel model;
   Q::connect(Log::instance(), &Log::newMessage, &model, [&](const QString & msg) {
      auto row = model.rowCount();
      model.insertRow(row);
      model.setData(model.index(row), msg);
   });
   QWidget w;
   QVBoxLayout layout{&w};
   QListView view;
   bool viewAtBottom = false;
   QPushButton clear{"Clear"};
   layout.addWidget(&view);
   layout.addWidget(&clear);
   Q::connect(&clear, &QPushButton::clicked,
              &model, [&]{ model.setStringList(QStringList{}); });
   view.setModel(&model);
   view.setUniformItemSizes(true);
   Q::connect(view.model(), &QAbstractItemModel::rowsAboutToBeInserted, &view, [&] {
      auto bar = view.verticalScrollBar();
      viewAtBottom = bar ? (bar->value() == bar->maximum()) : false;
   });
   Q::connect(view.model(), &QAbstractItemModel::rowsInserted,
              &view, [&]{ if (viewAtBottom) view.scrollToBottom(); });

   QtConcurrent::run([]{
      auto delay = 10;
      for (int ms = 0; ms <= 500; ms += delay) {
         r_printf("%d ms", ms);
         QThread::msleep(ms);
      }
   });
   w.show();
   return app.exec();
}
