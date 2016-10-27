// https://github.com/KubaO/stackoverflown/tree/master/questions/dynamic-widget-10790454
#include <cmath>
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif
#include <array>

//

class ClockView : public QLabel
{
public:
    explicit ClockView(QWidget* parent = nullptr) : QLabel(parent)
    {
        static int ctr = 0;
        setText(QString::number(ctr++));
    }
};

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void populateViewGrid();

private:
    static constexpr int N = 10;

    QWidget central{this};
    QGridLayout centralLayout{&central};
    std::array<QFrame, N> frames;

    std::array<ClockView, N> clockViews;
    std::array<QVBoxLayout, N> layouts;
};

//

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setCentralWidget(&central);

    const int n = ceil(sqrt(N));
    for (int i = 0; i < N; ++ i) {
        frames[i].setFrameShape(QFrame::StyledPanel);
        centralLayout.addWidget(&frames[i], i/n, i%n, 1, 1);
    }

    populateViewGrid();
}

void MainWindow::populateViewGrid()
{
    for (int i = 0; i < N; ++ i) {
        layouts[i].addWidget(&clockViews[i]);
        frames[i].setLayout(&layouts[i]);
    }
}

int main(int argc, char** argv)
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
