// https://github.com/KubaO/stackoverflown/tree/master/questions/simple-button-main-41729401
#include <QtWidgets>

class Ui_MainWindow {
public:
   QWidget *central;
   QGridLayout *layout;
   QLabel *label;
   void setupUi(QMainWindow *parent);
};

class MainWindow : public QMainWindow, private Ui_MainWindow {
   Q_OBJECT
   QPushButton m_button{"Click Me"};
public:
   MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
      setupUi(this);
      m_button.setParent(centralWidget());
      m_button.setGeometry({{50, 50}, m_button.sizeHint()});
   }
};

void Ui_MainWindow::setupUi(QMainWindow *parent) {
   central = new QWidget{parent};
   layout = new QGridLayout{central};
   label = new QLabel{"Hello"};
   label->setAlignment(Qt::AlignCenter);
   label->setStyleSheet("background-color:blue; color:white;");
   layout->addWidget(label, 0, 0);
   parent->setCentralWidget(central);
   parent->setMinimumSize(200, 200);
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   MainWindow w;
   w.show();
   return app.exec();
}
#include "main.moc"
