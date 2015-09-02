// CoordinateDialog.h
#include <QDialog>
#include <QVector3D>

class CoordinateDialogPrivate;
class CoordinateDialog : public QDialog
{
   Q_OBJECT
   Q_DECLARE_PRIVATE(CoordinateDialog)
#if QT_VERSION <= QT_VERSION_CHECK(5,0,0)
   Q_PRIVATE_SLOT(d_func(), void onAccepted())
#endif
   QScopedPointer<CoordinateDialogPrivate> const d_ptr;
public:
   CoordinateDialog(QWidget * parent = 0, Qt::WindowFlags flags = 0);
   ~CoordinateDialog();
   QVector3D coordinates() const;
   Q_SIGNAL void acceptedCoordinates(const QVector3D &);
};
Q_DECLARE_METATYPE(QVector3D)

// CoordinateDialog.cpp
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>

class CoordinateDialogPrivate {
   Q_DISABLE_COPY(CoordinateDialogPrivate)
   Q_DECLARE_PUBLIC(CoordinateDialog)
   CoordinateDialog * const q_ptr;
   QFormLayout layout;
   QDoubleSpinBox x, y, z;
   QDialogButtonBox buttons;
   QVector3D coordinates;
   void onAccepted();
   CoordinateDialogPrivate(CoordinateDialog*);
};

CoordinateDialogPrivate::CoordinateDialogPrivate(CoordinateDialog * dialog) :
   q_ptr(dialog),
   layout(dialog),
   buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
{
   layout.addRow("X", &x);
   layout.addRow("Y", &y);
   layout.addRow("Z", &z);
   layout.addRow(&buttons);
   dialog->connect(&buttons, SIGNAL(accepted()), SLOT(accept()));
   dialog->connect(&buttons, SIGNAL(rejected()), SLOT(reject()));
#if QT_VERSION <= QT_VERSION_CHECK(5,0,0)
   dialog->connect(dialog, SIGNAL(accepted()), SLOT(onAccepted()));
#else
   QObject::connect(dialog, &QDialog::accepted, [this]{ onAccepted(); });
#endif
}

void CoordinateDialogPrivate::onAccepted() {
   Q_Q(CoordinateDialog);
   coordinates.setX(x.value());
   coordinates.setY(y.value());
   coordinates.setZ(z.value());
   emit q->acceptedCoordinates(coordinates);
}

CoordinateDialog::CoordinateDialog(QWidget * parent, Qt::WindowFlags flags) :
   QDialog(parent, flags),
   d_ptr(new CoordinateDialogPrivate(this))
{}

QVector3D CoordinateDialog::coordinates() const {
   Q_D(const CoordinateDialog);
   return d->coordinates;
}

CoordinateDialog::~CoordinateDialog() {}

// main.cpp

#include <QApplication>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   CoordinateDialog d;
   d.show();
   return a.exec();
}

// Integer.h
#include <algorithm>

class IntegerPrivate;
class Integer {
   Q_DECLARE_PRIVATE(Integer)
   QScopedPointer<IntegerPrivate> d_ptr;
public:
   Integer();
   Integer(int);
   Integer(const Integer & other);
   Integer(Integer && other);
   operator int&();
   operator int() const;
   Integer & operator=(Integer other);
   friend void swap(Integer& first, Integer& second) /* nothrow */;
   ~Integer();
};

Integer::Integer(Integer && other) : Integer() {
   swap(*this, other);
}

Integer & Integer::operator=(Integer other) {
   swap(*this, other);
   return *this;
}

void swap(Integer& first, Integer& second) /* nothrow */ {
   using std::swap;
   swap(first.d_ptr, second.d_ptr);
}

// Integer.cpp
class IntegerPrivate {
public:
   int value;
   IntegerPrivate(int i) : value(i) {}
};

Integer::Integer() : d_ptr(new IntegerPrivate(0)) {}
Integer::Integer(int i) : d_ptr(new IntegerPrivate(i)) {}
Integer::Integer(const Integer &other) :
   d_ptr(new IntegerPrivate(other.d_func()->value)) {}
Integer::operator int&() { return d_func()->value; }
Integer::operator int() const { return d_func()->value; }
Integer::~Integer() {}


#include "main.moc"
