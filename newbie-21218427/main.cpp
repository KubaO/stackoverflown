// Interface
//
#include <QWidget>

class QLabel;
class QPushButton;
class QSpinBox;
class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int a, b, c;
private slots:
    void clearx();
    void equalsx();
    void addx();
    void subtractx();
    void multiplyx();
    void dividex();
    void firstnumberx();
    void secondnumberx();
private:
    QString value, total;
    int f, s;
    enum Operation { None, Add, Subtract, Multiply, Divide };
    Operation op;

    QLabel *label;
    QPushButton *equal;
    QPushButton *clear;
    QPushButton *equals;
    QPushButton *add;
    QPushButton *subtract;
    QPushButton *multiply;
    QPushButton *divide;
    QSpinBox *spinner;
    QSpinBox *spinner2;
};

// Implementation
//
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent), op(None)

{
    label = new QLabel("0",this);
    label -> setGeometry(QRect(QPoint(75,25),QSize(50,200)));

    clear = new QPushButton("Clear", this);
    clear  -> setGeometry(QRect(QPoint(80,300),QSize(50,50)));
    connect(clear,SIGNAL(released()),this,SLOT(clearx()));

    equal = new QPushButton("Equal", this);
    equal  -> setGeometry(QRect(QPoint(110,300),QSize(50,50)));
    connect(equal,SIGNAL(released()),this,SLOT(equalsx()));

    add = new QPushButton("Add", this);
    add -> setGeometry(QRect(QPoint(140,300),QSize(50,50)));
    connect(add,SIGNAL(released()),this,SLOT(addx()));

    subtract = new QPushButton("Subtract", this);
    subtract -> setGeometry(QRect(QPoint(170,300),QSize(50,50)));
    connect(subtract,SIGNAL(released()),this,SLOT(subtractx()));

    multiply = new QPushButton("Multiply", this);
    multiply -> setGeometry(QRect(QPoint(200,300),QSize(50,50)));
    connect(multiply,SIGNAL(released()),this,SLOT(multiplyx()));

    divide = new QPushButton("Divide", this);
    divide -> setGeometry(QRect(QPoint(230,300),QSize(50,50)));
    connect(divide,SIGNAL(released()),this,SLOT(dividex()));

    spinner = new QSpinBox(this);
    spinner -> setGeometry(QRect(QPoint(130,150),QSize(50,50)));
    connect(divide,SIGNAL(released()),this,SLOT(firstnumberx()));
    spinner->setRange(1,10);

    spinner2 = new QSpinBox(this);
    spinner2 -> setGeometry(QRect(QPoint(190,150),QSize(50,50)));
    connect(divide,SIGNAL(released()),this,SLOT(secondnumberx()));
    spinner2->setRange(1,10);
}

void MainWindow::clearx()
{
    value = "";
    label -> setText(value);
}

void MainWindow::equalsx()
{
    s = value.toInt();
    if (op == Add)
    {
        total = QString::number(f+s);
        label->setText(total);
    }
}

void MainWindow::addx()
{
    c = a + b;
    label -> setText(QString::number(c));
}

void MainWindow::firstnumberx()
{
    a = spinner->value();
}

void MainWindow::secondnumberx()
{
    b = spinner->value();
}

void MainWindow::subtractx() {}
void MainWindow::multiplyx() {}
void MainWindow::dividex() {}
MainWindow::~MainWindow() {}

int main(int argc, char ** argv)
{
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

#include "main.moc"
