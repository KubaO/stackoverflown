#include <QLabel>
#include <QStringList>
#include <QListView>
#include <QStringListModel>
#include <QAction>

class LabelWithHistory : public QLabel {
   Q_OBJECT
   Q_PROPERTY(QString text READ text WRITE setText)
   QStringList m_history;
   QStringListModel* m_model;
   QListView* m_view;
   void init() {
      QAction * showHistory = new QAction("History", this);
      connect(showHistory, SIGNAL(triggered()), SLOT(showHistory()));
      addAction(showHistory);
      setContextMenuPolicy(Qt::ActionsContextMenu);
   }
   void initView() {
      m_model = new QStringListModel(this);
      m_view = new QListView(this);
      m_view->setWindowFlags(Qt::Window);
      m_view->setModel(m_model);
      m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
      m_view->setSelectionMode(QAbstractItemView::NoSelection);
      m_view->setWordWrap(true);
      m_view->setAlternatingRowColors(true);
   }
   Q_SLOT void showHistory() {
      if (! m_view) initView();
      if (m_view->isVisible()) return;
      m_model->setStringList(m_history);
      m_view->move(mapToGlobal(QPoint(0, height())));
      QModelIndex lastRow(m_model->index(m_history.size()-1));
      int bottomY = m_view->visualRect(lastRow).bottom();
      int widthHint = m_view->sizeHint().width();
      if (bottomY < m_view->sizeHint().height())
         m_view->resize(widthHint, bottomY + 1);
      m_view->show();
   }
   void hideEvent(QHideEvent *) {
      if (m_view) m_view->hide();
   }
public:
   LabelWithHistory(QWidget * parent = 0, Qt::WindowFlags f = 0) :
      QLabel(parent,f), m_model(0), m_view(0) { init(); }
   LabelWithHistory(const QString & text, QWidget * parent = 0, Qt::WindowFlags f = 0) :
      QLabel(text, parent, f), m_model(0), m_view(0) { init(); }
   Q_SLOT void setText(const QString & text) {
      if (text == this->text()) return;
      m_history.prepend(text);
      if (m_view && m_view->isVisible())
         m_model->setStringList(m_history);
      QLabel::setText(text);
   }
};

#include <QApplication>
#include <QHBoxLayout>
#include <QPushButton>

int main(int argc, char ** argv)
{
   auto items = QStringList() << "Foo" << "Bar" << "Baz" << "Ban";
   QApplication a(argc, argv);
   QWidget w;
   QHBoxLayout layout(&w);
   LabelWithHistory label;
   QPushButton button("Add Item");
   QObject::connect(&button, &QPushButton::clicked, [&label, &items](){
      label.setText(items[qrand() % items.size()]);
   });
   layout.addWidget(&label);
   layout.addWidget(&button);
   for (auto item : items) label.setText(item);
   w.setMinimumSize(200, 50);
   w.show();
   return a.exec();
}

#include "main.moc"
