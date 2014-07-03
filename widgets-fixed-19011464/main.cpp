#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QLabel * lbl1 = new QLabel("one");
    lbl1->setStyleSheet("QLabel { background-color: #FF8080 }");
    lbl1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    QLabel * lbl2 = new QLabel("two");
    lbl2->setStyleSheet("QLabel { background-color: #80FF80 }");
    lbl2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    lbl2->setMinimumHeight(300);

    QLabel * lbl3 = new QLabel("three");
    lbl3->setStyleSheet("QLabel { background-color: #8080FF }");
    lbl3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    lbl3->setMinimumWidth(300);

    QWidget window;
    QHBoxLayout * layout = new QHBoxLayout(&window);
    layout->addWidget(lbl1);
    layout->addWidget(lbl2);
    layout->addWidget(lbl3);
    window.setMinimumSize(800, 800);
    // Any combination of setMinimumSize and setMaximumSize is OK.
    // If the minimum and maximum are the same, just do setFixedSize

    QFontDatabase db;
    QFont const font = window.font();
    QStringList const families = db.families();
    Q_ASSERT(families.contains(font.family()) || false);
    QFontInfo const info(font);
    Q_ASSERT(families.contains(info.family()));

    window.show();
    return a.exec();
}

