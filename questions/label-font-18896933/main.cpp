// https://github.com/KubaO/stackoverflown/tree/master/questions/label-font-18896933
// This project is compatible with Qt 4 and Qt 5
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets>
#endif

bool isFixedPitch(const QFont &font) {
   const QFontInfo fi(font);
   qDebug() << fi.family() << fi.fixedPitch();
   return fi.fixedPitch();
}

QFont getMonospaceFont() {
   QFont font("monospace");
   if (isFixedPitch(font)) return font;
   font.setStyleHint(QFont::Monospace);
   if (isFixedPitch(font)) return font;
   font.setStyleHint(QFont::TypeWriter);
   if (isFixedPitch(font)) return font;
   font.setFamily("courier");
   if (isFixedPitch(font)) return font;
   return font;
}

int main(int argc, char *argv[]) {
   QApplication a(argc, argv);
   QString text("0123456789ABCDEF");
   QWidget w;
   QVBoxLayout layout(&w);
   QLabel label1(text), label2(text);
   label1.setFont(getMonospaceFont());
   layout.addWidget(&label1);
   layout.addWidget(&label2);
   w.show();
   return a.exec();
}
