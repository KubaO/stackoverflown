// https://github.com/KubaO/stackoverflown/tree/master/questions/click-sim-win-43732690
#include <QtWidgets>
#include <Windows.h>

template <typename T> class Named : public T { public:
   template <typename... ARGS> Named(const QString & name, ARGS&&... args) :
      T(std::forward<ARGS>(args)...) { setObjectName(name); }
   template <typename... ARGS> Named(const QString & name, QKeySequence && shortcut, ARGS&&... args) :
      T(std::forward<ARGS>(args)...) {
      setObjectName(name);
      setShortcut(shortcut);
      auto t = text();
      if (!t.contains("%1")) t += " (%1)";
      setText(t.arg(shortcut.toString(QKeySequence::NativeText)));
   }
};
#define OBJ(T, name, ...) Named<T> name{#name, __VA_ARGS__}

class MainWindow : public QWidget
{
   Q_OBJECT
   using K = QKeySequence;
   QGridLayout layout{this};
   QTextEdit speed_label;
   OBJ(QPushButton, repeat, K{"Shift+R"}, "REPEAT");
   OBJ(QPushButton, points, K{"Ctrl+P"}, "CHOOSE POINTS (%1 then ,)");
   OBJ(QPushButton, start, K{"Shift+S"}, "START");
   OBJ(QLineEdit, xy);
   OBJ(QRadioButton, hide, K{"Alt+H"}, "HIDE");
   OBJ(QPlainTextEdit, list);
   OBJ(QPushButton, stop, K{"Shift+Space"}, "STOP");
   OBJ(QPushButton, repeat_off, K{"Shift+F"}, "REPEAT OFF");
   OBJ(QRadioButton, off, K{"Shift+O"}, "OFF");
   OBJ(QPushButton, speed, K{"Shift+A"}, "SET SPEED");
   OBJ(QPushButton, clear, K{"Shift+C"}, "CLEAR");
   OBJ(QPushButton, click_on, K{"Shift+M"}, "MOUSE CLICK ON/OFF");

   OBJ(QTimer, timer, this);          //timer for capture a cursor
   OBJ(QTimer, timer_move, this);     //timer for set cursor position
   OBJ(QTimer, timer_points, this);   //timer for certain points
   QPoint p;

   std::vector<QPoint> poz;
   int i=0;
   int a=0;
   double timer_time=10;
   bool certain=false;
   bool mouse_clicked=true;

   void keyReleaseEvent(QKeyEvent *e) override;

private slots:
   void on_start_clicked();
   void on_stop_clicked();
   void on_timer_timeout();
   void on_hide_clicked();
   void on_off_clicked();
   void on_repeat_clicked();
   void on_timer_move_timeout();
   void on_repeat_off_clicked();
   void on_points_clicked(); //choose points with tab
   void on_timer_points_timeout();
   void on_clear_clicked();
   void on_speed_clicked();
   void on_speed_label_textChanged();
   void on_click_on_clicked();

public:
   explicit MainWindow(QWidget *parent = {}) : QWidget(parent) {
      layout.addWidget(&hide, 1, 0);
      layout.addWidget(&off, 1, 1);
      layout.addWidget(&start, 2, 0);
      layout.addWidget(&stop, 2, 1);
      layout.addWidget(&points, 3, 0);
      layout.addWidget(&xy, 3, 1);
      layout.addWidget(&list, 6, 0, 3, 1);
      layout.addWidget(&repeat, 6, 1);
      layout.addWidget(&repeat_off, 7, 1);
      layout.addWidget(&clear,  8, 1);
      layout.addWidget(&speed, 10, 0);
      layout.addWidget(&speed_label, 10, 1);
      layout.addWidget(&click_on, 11, 0);

      setStyleSheet("background-color: #535353;" "color: white;");
      click_on.setStyleSheet("color: #ceb008;");
      speed_label.setText(QString::number(timer_time));

      //setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      QMetaObject::connectSlotsByName(this);
   }
};

void MainWindow::on_start_clicked()
{
   if (!certain) timer.start(timer_time);
   else timer_points.start(timer_time);

}

void MainWindow::on_stop_clicked()
{
   timer.stop();
}

void MainWindow::on_timer_timeout()
{
   p = QCursor::pos();
   poz.push_back(p);
   xy.setText("x: "+QString::number(p.rx())+"  y: "+QString::number(p.ry()));
   list.appendPlainText("x: "+QString::number(poz[i].rx())+"  y: "+QString::number(poz[i].ry()));
   i++;
}

void MainWindow::on_hide_clicked()
{
   setWindowFlags(Qt::WindowStaysOnBottomHint);
}

void MainWindow::on_off_clicked()
{
   exit(0);
}

void MainWindow::on_repeat_clicked()
{
   repeat.setStyleSheet("color: #ceb008;");
   on_timer_move_timeout();
}

void MainWindow::on_timer_move_timeout(){
   //int x=1027;
   //int y=322;
   int x=rand()%1920;
   int y=rand()%1080;
   on_stop_clicked();
   if(a==0) {timer_move.start(timer_time);}
   if(a<i) {
      cursor().setPos(poz[a].rx(), poz[a].ry());
      // QPointF p(rand()%1920,rand()%1080);
      // QMouseEvent(QEvent::MouseButtonPress, p, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
      // QMouseEvent(QEvent::MouseButtonRelease, p, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
      //QTest::mousePress(this,);
      // QTest::mouseRelease();
      //QTest::mouseClick(this,Qt::LeftButton,Qt::NoModifier,poz[a]);
      // QTest::mouseClick (this, Qt::LeftButton, Qt::NoModifier, QPoint(660, 1060), 100);
      //QWidget *d = QApplication::desktop().screen();
      //QTest::mouseClick(d, Qt::LeftButton, Qt::NoModifier, QPoint(x,y));


      HWND h = GetForegroundWindow();

      WORD mouseX = 10;// x coord of mouse

      WORD mouseY = 10;// y coord of mouse

      PostMessage(h,WM_LBUTTONDOWN,0,MAKELPARAM(mouseX,mouseY));
      a++;}
   else a=0;
}

void MainWindow::on_repeat_off_clicked()
{
   timer_move.stop();
   a=0;

   repeat.setStyleSheet("color: white;");
}

void MainWindow::on_points_clicked()
{
   if (certain==false) {
      certain=true;
      points.setStyleSheet("color: #ceb008;");
   }
   else {
      certain=false;
      points.setStyleSheet("color: white;");
   }
}

void MainWindow::keyReleaseEvent(QKeyEvent *e){
   switch(e->key()){
   case Qt::Key_Comma:
      qDebug() << e;
      on_timer_timeout();
      e->accept();
      break;
   }
   QWidget::keyReleaseEvent(e);
}

void MainWindow::on_timer_points_timeout(){
   p = QCursor::pos();
   xy.setText("x: "+QString::number(p.rx())+"  y: "+QString::number(p.ry()));
}

void MainWindow::on_clear_clicked()
{
   timer.stop();
   timer_move.stop();
   poz.erase(poz.begin(),poz.end());
   i=0;
   a=0;
   timer_time=10;
   certain=false;
   xy.setText("");
   list.clear();

}
void MainWindow::on_speed_clicked()
{
   speed_label.setReadOnly(false);
   speed.setStyleSheet("color: #ceb008;");
}

void MainWindow::on_speed_label_textChanged()
{
   speed_label.setReadOnly(true);
   timer_time=(speed_label.toPlainText()).toDouble();
   speed.setStyleSheet("color: white;");

}

void MainWindow::on_click_on_clicked()
{
   if (mouse_clicked) {
      click_on.setStyleSheet("color: white;");
      mouse_clicked=false;
   }
   else {
      click_on.setStyleSheet("color: #ceb008;");
      mouse_clicked=true;}
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   MainWindow w;
   w.show();
   return app.exec();
}

#include "main.moc"
