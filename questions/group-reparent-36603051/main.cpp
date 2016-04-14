// https://github.com/KubaO/stackoverflown/tree/master/questions/group-reparent-36603051
#include <QtWidgets>

/// Replaces the visible widget with a hidden widget, preserving the layout of the
/// children, and making the new widget visible.
void swapWidgets(QWidget * a, QWidget * b)
{
   auto src = a->isVisible() ? a : b;
   auto dst = a->isVisible() ? b : a;
   Q_ASSERT(dst->isHidden());
   /// Move the children to the destination
   dst->setLayout(src->layout());
   /// Replace source with destination in the parent
   auto layout = src->parentWidget()->layout();
   delete layout->replaceWidget(src, dst);
   /// Unparent the source, otherwise it won't be reinsertable into the parent.
   src->setParent(nullptr);
   /// Only the destination should be seen.
   src->hide();
   dst->show();
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QWidget w;
   QGridLayout wLayout{&w};
   QPushButton swapBtn{"Swap"};
   wLayout.addWidget(&swapBtn);

   QWidget noBox;
   QGroupBox box{"Group"};
   wLayout.addWidget(&box);
   QGridLayout boxLayout{&box};
   for (int i = 0; i < 16; ++i)
      boxLayout.addWidget(new QLabel(QString("Tr%1").arg(i)), i/8, i%8);

   swapBtn.connect(&swapBtn, &QPushButton::clicked, [&] { swapWidgets(&box, &noBox); });
   w.show();
   return app.exec();
}

