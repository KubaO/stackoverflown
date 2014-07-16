#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QBoxLayout>

class Calculator : public QWidget {
   Q_OBJECT
public:
   Calculator();

private slots:
   void on_addNumber_clicked();

public:
   QSpinBox *firstValueSpinBox;
   QSpinBox *secondValueSpinBox;
   QLabel *resultLabel;
};

Calculator::Calculator(){
   QPushButton *addButton = new QPushButton("Add");
   firstValueSpinBox = new QSpinBox();
   secondValueSpinBox = new QSpinBox();
   resultLabel = new QLabel();
   QLabel *addLabel = new QLabel("+");
   QLabel *equalLabel = new QLabel("=");

   connect(addButton, SIGNAL(clicked()), this, SLOT(on_addNumber_clicked()));

   QHBoxLayout *layout = new QHBoxLayout(this);
   layout->addWidget(firstValueSpinBox);
   layout->addWidget(addLabel);
   layout->addWidget(secondValueSpinBox);
   layout->addWidget(addButton);
   layout->addWidget(equalLabel);
   layout->addWidget(resultLabel);
}

void Calculator::on_addNumber_clicked(){
   int num = this->firstValueSpinBox->value();
   int num2 = this->secondValueSpinBox->value();
   QString outResult = QString::number(num + num2);
   resultLabel->setText(outResult);       //<< the problem here
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Calculator w;
    w.show();
    return a.exec();
}

#include "main.moc"
