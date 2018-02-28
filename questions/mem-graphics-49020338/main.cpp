// https://github.com/KubaO/stackoverflown/tree/master/questions/mem-graphics-49020338

// Dummy UI
#include <QPushButton>
#include <QGraphicsView>

namespace Ui {
class GuiApplicationClass {
public:
   QGraphicsView * graphicsView;
   QPushButton * btn_Display;
   void setupUi(QWidget * w) {
      graphicsView = new QGraphicsView(w);
      btn_Display = new QPushButton(w);
      btn_Display->setObjectName("btn_Display");
      QMetaObject::connectSlotsByName(w);
   }
};
}

// Interface
#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include "ui_QtGuiApplication.h"

class GuiApplication : public QMainWindow
{
   Q_OBJECT

public:
   GuiApplication(QWidget *parent = {});

private:
   Ui::GuiApplicationClass ui;

   QGraphicsScene scene;
   QGraphicsPathItem pathItem; // the order matters: must be declared after the scene
   int index_ = 0;

   Q_SLOT void btn_Display_clicked();
};

// Implementation

#include "GuiApplication.h"
#define _USE_MATH_DEFINES
#include <cmath>

#if !defined(M_PI)
#define M_PI (3.14159265358979323846)
#endif

GuiApplication::GuiApplication(QWidget *parent) : QMainWindow(parent)
{
   ui.setupUi(this);
   ui.graphicsView->setScene(&scene);
   ui.graphicsView->show();
   pathItem.setPen({Qt::red, 2});
   scene.addItem(&pathItem);
}

#if 0
void GuiApplication::btn_Display_clicked()
{
   const double phi_ = 0;
   const double freq_ = 5;
   const int N = 800;
   QPolygonF pol{N};
   for (int i = 0; i < pol.size(); ++i) {
      qreal x = i;
      qreal y = 100 * sin(2 * M_PI * freq_ * i / 811 + phi_) + 20 * index_;
      pol[i] = {x, y};
   }
   QPainterPath path;
   path.addPolygon(pol);
   pathItem.setPath(path);
   index_ = (index_ + 1) % 20; // just for sense of change in graph
}
#else
void GuiApplication::btn_Display_clicked()
{
   const double phi_ = 0;
   const double freq_ = 5;
   const int N = 800;
   if (pathItem.path().isEmpty()) { // preallocate the path
      QPainterPath path;
      path.addPolygon(QPolygon{N});
      pathItem.setPath(path);
   }
   auto path = pathItem.path();
   pathItem.setPath({}); // we own the path now - item's path is detached
   Q_ASSERT(path.elementCount() == N);
   for (int i = 0; i < path.elementCount(); ++i) {
      qreal x = i;
      qreal y = 100 * sin(2 * M_PI * freq_ * i / 811 + phi_) + 20 * index_;
      path.setElementPositionAt(i, x, y);
   }
   pathItem.setPath(path);
   index_ = (index_ + 1) % 20; // just for sense of change in graph
}
#endif

#include <QtWidgets>
int main(int argc, char ** argv) {
   QApplication app(argc, argv);
   GuiApplication g;
   QMetaObject::invokeMethod(&g, "btn_Display_clicked");
   QMetaObject::invokeMethod(&g, "btn_Display_clicked");
}

#include "main.moc"
