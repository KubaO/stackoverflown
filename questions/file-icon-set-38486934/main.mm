// https://github.com/KubaO/stackoverflown/tree/master/questions/file-icon-set-38486934
#include <QtWidgets>
#include <QtMacExtras>
#include <AppKit/AppKit.h>

QPixmap circle() {
   QPixmap pix{128, 128};
   pix.fill(Qt::transparent);
   QPainter p{&pix};
   p.setBrush(Qt::yellow);
   p.setPen(QPen{Qt::blue, 3});
   p.drawEllipse(pix.rect());
   return pix;
}

NSString * toNSString(const QString & str) {
   return const_cast<NSString*>(reinterpret_cast<const NSString*>(str.toCFString()));
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   auto path = QFileDialog::getOpenFileName(nullptr, "Select File");
   if (! path.isEmpty()) {
      auto image = QtMac::toNSImage(circle());
      auto filePath = toNSString(path);
      [[NSWorkspace sharedWorkspace] setIcon:image forFile:filePath options:0];
      [filePath release];
      [image release];
   }
   return 0;
}
