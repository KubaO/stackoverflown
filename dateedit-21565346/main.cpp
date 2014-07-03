#include <QApplication>
#include <QDateEdit>
#include <QDate>
#include <QHBoxLayout>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWidget w;
    QHBoxLayout * layout = new QHBoxLayout(&w);
    QDateEdit edit;
    edit.setDate(QDate::currentDate());
    edit.setCalendarPopup(true);
    edit.setDisplayFormat("MM/dd/yyyy");
    layout->addWidget(&edit);
    w.show();
    return a.exec();
}
