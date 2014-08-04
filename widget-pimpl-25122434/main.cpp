// Interface (header file)

#include <QWidget>

class QDate;
class GUICalendarDayPrivate;
class GUICalendarDay : public QWidget {
   Q_OBJECT
   Q_DECLARE_PRIVATE(GUICalendarDay)
   QScopedPointer<GUICalendarDayPrivate> const d_ptr;
public:
   explicit GUICalendarDay(const QDate & date, QWidget * parent = 0);
   ~GUICalendarDay();
};

// Implementation (.cpp file)

#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QDate>

// dummy to make the example compile
class GUICalendarCell : public QWidget {
   Q_OBJECT
public:
   explicit GUICalendarCell(const QTime &, const QTime &, QWidget * parent = 0) : QWidget(parent) {}
   Q_SIGNAL void clickedRange(const QTime &, const QTime &);
};

class GUICalendarDayPrivate {
public:
   QDate date;
   QGridLayout layout;
   QPushButton prevDay, nextDay;
   QLabel dateLabel;
   QList<GUICalendarCell*> schedule;

   QList<QLabel*> hourLabels;
   explicit GUICalendarDayPrivate(const QDate &, QWidget * parent);
   void onNextDay() {}
   void onPrevDay() {}
   void selectedRange(const QTime &, const QTime &) {}
};

GUICalendarDay::GUICalendarDay(const QDate &date, QWidget *parent) : QWidget(parent),
   d_ptr(new GUICalendarDayPrivate(date, this))
{}

GUICalendarDay::~GUICalendarDay() {}

GUICalendarDayPrivate::GUICalendarDayPrivate(const QDate & date_, QWidget * parent) :
   date(date_),
   layout(parent),
   prevDay("<"), nextDay(">"), dateLabel(date.toString(QString("dd/MM/yyyy")))
{
   const int TOTALHOURS = 24;
   QObject::connect(&nextDay, &QAbstractButton::clicked, [this]{ onNextDay(); });
   QObject::connect(&prevDay, &QAbstractButton::clicked, [this]{ onPrevDay(); });

   layout.addWidget(&prevDay, 0, 1);
   layout.addWidget(&dateLabel, 0, 2);
   layout.addWidget(&nextDay, 0, 3);

   for(int i = 0, row = layout.rowCount(); i<TOTALHOURS; ++i, ++row) {
      auto from = QTime(i, 0, 0), to = QTime(i, 59, 59);
      auto cell = new GUICalendarCell(from, to);
      auto hourLabel = new QLabel(from.toString(QString("hh:mm")));

      QObject::connect(cell, &GUICalendarCell::clickedRange,
                       [this](const QTime & from, const QTime & to){
         selectedRange(from, to);
      });

      schedule << cell;
      hourLabels << hourLabel;
      layout.addWidget(hourLabel, row, 0);
      layout.addWidget(cell, row, 1, 3, 1);
   }
}

// main.cpp
#include <QApplication>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   GUICalendarDay day(QDate::currentDate());
   day.show();
   return a.exec();
}

// Only needed if all of the example is in a single file
#include "main.moc"
