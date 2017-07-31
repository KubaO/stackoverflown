// https://github.com/KubaO/stackoverflown/tree/master/questions/style-font-query-test-45422885
#include <QtWidgets>

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QLabel label("Test");
   auto font1 = label.font();
   label.setStyleSheet("font-size: 49pt;");
   label.show();
   label.ensurePolished();
   auto font2 = label.font();
   Q_ASSERT(font1.pointSize() != 49);
   Q_ASSERT(font2.pointSize() == 49);
   Q_ASSERT(font1.family() == font2.family());
}
