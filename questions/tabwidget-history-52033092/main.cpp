// https://github.com/KubaO/stackoverflown/tree/master/questions/tabwidget-history-52033092
#include <QtWidgets>
#include <array>

static const char kHistory[] = "history";

auto getHistory(const QTabWidget *w) {
   return w->property(kHistory).value<QList<int>>();
}

void addHistory(QTabWidget *tabWidget) {
   QObject::connect(tabWidget, &QTabWidget::currentChanged, [tabWidget](int index) {
      if (index < 0) return;
      auto history = getHistory(tabWidget);
      history.removeAll(index);
      history.append(index);
      tabWidget->setProperty(kHistory, QVariant::fromValue(history));
   });
   if (tabWidget->currentIndex() >= 0)
      tabWidget->setProperty(
          kHistory, QVariant::fromValue(QList<int>() << tabWidget->currentIndex()));
}

bool hasVisitedPage(const QTabWidget *w, int index) {
   return getHistory(w).contains(index);
}

int main(int argc, char *argv[]) {
   QApplication app(argc, argv);
   QWidget ui;
   QVBoxLayout layout{&ui};
   QTabWidget tabWidget;
   QLabel history;
   layout.addWidget(&tabWidget);
   layout.addWidget(&history);
   std::array<QLabel, 5> tabs;
   for (auto &l : tabs) {
      auto const n = &l - &tabs[0] + 1;
      l.setText(QStringLiteral("Label on Page #%1").arg(n));
      tabWidget.addTab(&l, QStringLiteral("Page #%1").arg(n));
   }
   addHistory(&tabWidget);
   auto showHistory = [&] {
      auto text = QStringLiteral("History: ");
      for (auto i : tabWidget.property("history").value<QList<int>>())
         text.append(QString::number(i + 1));
      history.setText(text);
   };
   showHistory();
   QObject::connect(&tabWidget, &QTabWidget::currentChanged, showHistory);
   tabWidget.currentChanged(tabWidget.currentIndex());
   ui.show();
   return app.exec();
}
