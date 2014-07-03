#include <QApplication>
#include <QStringListModel>
#include <QListView>
#include <QPainter>
#include <QFormLayout>
#include <QSpinBox>

class ListView : public QListView {
    void paintEvent(QPaintEvent *e) {
        QListView::paintEvent(e);
        if (model() && model()->rowCount(rootIndex())) return;
        // The view is empty.
        QPainter p(this->viewport());
        p.drawText(rect(), Qt::AlignCenter, "No Items");
    }
public:
    ListView(QWidget* parent = 0) : QListView(parent) {}
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWidget window;
    QFormLayout * l = new QFormLayout(&window);
    ListView * view = new ListView;
    QSpinBox * spin = new QSpinBox;
    QStringListModel * model = new QStringListModel(&window);
    l->addRow(view);
    l->addRow("Item Count", spin);
    QObject::connect(spin, (void (QSpinBox::*)(int))&QSpinBox::valueChanged,
    [=](int value){
        QStringList list;
        for (int i = 0; i < value; ++i) list << QString("Item %1").arg(i);
        model->setStringList(list);
    });
    view->setModel(model);
    window.show();
    return a.exec();
}
