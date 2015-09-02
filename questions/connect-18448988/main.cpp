//main.cpp
#include <QApplication>
#include <QGridLayout>
#include <QSplitter>
#include <QPushButton>
#include <QMessageBox>

class Other : public QObject
{
    Q_OBJECT
    QWidget *m_widget;
    Q_SLOT void testFunction() {
        QMessageBox::information(NULL, "Test", "Slot works.");
    }
public:
    QWidget *widget() const { return m_widget; }
    Other(QObject *parent = 0) : m_widget(new QWidget), QObject(parent) {
        QGridLayout *l = new QGridLayout(m_widget);
        QPushButton *btn = new QPushButton("Test Button");
        l->addWidget(btn);
        connect(btn, SIGNAL(clicked()), this, SLOT(testFunction()));
    }
    ~Other() { if (!m_widget->parent()) delete m_widget; }
};

class Main : public QWidget
{
public:
    Main(QWidget *parent = 0, Qt::WindowFlags f = 0) : QWidget(parent, f) {
        QGridLayout *l = new QGridLayout(this);
        QPushButton *btn = new QPushButton("Delete Other");
        Other *o = new Other(this);
        QSplitter *spl = new QSplitter;
        spl->addWidget(o->widget());
        l->addWidget(spl);
        l->addWidget(btn);
        connect(btn, SIGNAL(clicked()), o, SLOT(deleteLater()));
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Main w;
    w.show();
    return a.exec();
}

#include "main.moc"
