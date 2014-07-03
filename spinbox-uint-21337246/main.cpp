#include <QApplication>
#include <QLineEdit>
#include <QSpinBox>

class UnsignedSpinBox : public QSpinBox {
private:
    uint32_t value = 0;
    uint32_t minimum = 0;
    uint32_t maximum = 100;
private:
    void stepBy(int steps) {
        if (steps < 0 && (uint32_t)(-1 * steps) > value - minimum)
            value = minimum;
        else if (steps > 0 && maximum - value < (uint32_t)steps)
            value = maximum;
        else
            value += steps;
        lineEdit()->setText(QString::number(value));
    }
    StepEnabled stepEnabled() const {
        if (value < maximum && value > minimum)
            return QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
        else if (value < maximum)
            return QAbstractSpinBox::StepUpEnabled;
        else if (value > minimum)
            return QAbstractSpinBox::StepDownEnabled;
        else
            return QAbstractSpinBox::StepNone;
    }
    QValidator::State validate(QString &input, int &) const {
        if (input.isEmpty())
            return QValidator::Intermediate;
        bool ok = false;
        uint32_t validateValue = input.toUInt(&ok);
        if (!ok || validateValue > maximum || validateValue < minimum)
            return QValidator::Invalid;
        else
            return QValidator::Acceptable;
    }

public:
    UnsignedSpinBox(QWidget* parent = 0) : QAbstractSpinBox(parent) {
        lineEdit()->setText(QString::number(value));
    }
    virtual ~UnsignedSpinBox() { }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UnsignedSpinBox box;
    box.setMinimumSize(200,200);
    box.show();
    return a.exec();
}
