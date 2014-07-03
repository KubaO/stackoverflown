#include <QLabel>
#include <QStringList>
#include <QListView>
#include <QStringListModel>
#include <QAction>
#include <QScopedPointer>

class LabelWithHistory : public QLabel {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    QStringList m_history;
    QStringListModel m_model;
    QScopedPointer<QListView> m_view;
    void init() {
        QAction * showHistory = new QAction("History", this);
        connect(showHistory, SIGNAL(triggered()), SLOT(showHistory()));
        addAction(showHistory);
        setContextMenuPolicy(Qt::ActionsContextMenu);
    }
    void initView() {
        m_view.reset(new QListView);
        m_view->setModel(&m_model);
        m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_view->setSelectionMode(QAbstractItemView::NoSelection);
        m_view->setWordWrap(true);
        m_view->setAlternatingRowColors(true);
    }
    Q_SLOT void showHistory() {
        if (! m_view) initView();
        if (m_view->isVisible()) return;
        m_model.setStringList(m_history);
        m_view->move(mapToGlobal(QPoint(0, height())));
        QModelIndex lastRow(m_model.index(m_history.size()-1));
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
    LabelWithHistory(QWidget * parent = 0, Qt::WindowFlags f = 0) : QLabel(parent,f) {
        init();
    }
    LabelWithHistory(const QString & text, QWidget * parent = 0, Qt::WindowFlags f = 0) :
        QLabel(text, parent, f) { init(); }
    Q_SLOT void setText(const QString & text) {
        if (text == this->text()) return;
        m_history.prepend(text);
        if (m_view && m_view->isVisible())
            m_model.setStringList(m_history);
        QLabel::setText(text);
    }
};

#include <QApplication>
#include <QHBoxLayout>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWidget w;
    QLayout * layout = new QHBoxLayout(&w);
    LabelWithHistory * label = new LabelWithHistory;
    QPushButton * button = new QPushButton("Add Item");
    QObject::connect(button, &QPushButton::clicked, [=](){
        static QString items[3] = { "Foo", "Bar", "Baz"};
        label->setText(items[qrand() % 3]);
    });
    layout->addWidget(label);
    layout->addWidget(button);

    label->setText("Foo");
    label->setText("Bar");
    label->setText("Baz");
    w.setMinimumSize(200, 50);
    w.show();
    return a.exec();
}

#include "main.moc"
