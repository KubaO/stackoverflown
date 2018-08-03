// https://github.com/KubaO/stackoverflown/tree/master/questions/ascii-add-ui-51639347
#include <QtWidgets>

int main(int argc, char *argv[]) {
   QApplication a(argc, argv);
   QWidget ui;
   QFormLayout layout(&ui);
   QLineEdit input("Hello, World!");
   QLabel output;
   QLineEdit offset;
   QIntValidator offsetValidator(-65535, 65535);
   QCheckBox ascii("Modulo printable ASCII range 32-127");
   output.setFrameShape(QFrame::Panel);
   layout.addRow("Input Text", &input);
   layout.addRow("Output", &output);
   layout.addRow("Offset", &offset);
   layout.addRow(&ascii);
   layout.setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
   offset.setValidator(&offsetValidator);
   offset.setPlaceholderText("0");
   ascii.setChecked(true);
   ui.show();

   auto const calculate = [&] {
      bool limit = ascii.isChecked();
      auto text = input.text();
      int delta = offset.text().toInt();
      for (QChar &ch : text) {
         auto doLimit = limit && ch >= 32 && ch <= 127;
         ch = {ch.unicode() + delta};
         if (doLimit) ch = {((ch.unicode() - 32) % (128 - 32)) + 32};
      }
      output.setText(text);
   };
   for (auto ed : {&input, &offset})
      QObject::connect(ed, &QLineEdit::textChanged, calculate);
   QObject::connect(&ascii, &QCheckBox::toggled, calculate);

   offset.setText("1");
   return a.exec();
}
