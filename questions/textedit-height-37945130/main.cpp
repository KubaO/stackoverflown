// https://github.com/KubaO/stackoverflown/tree/master/questions/textedit-height-37945130
#include <QtWidgets>

void updateSize(QTextEdit * edit) {
   auto textHeight = edit->document()->documentLayout()->documentSize().height();
   edit->setFixedHeight(textHeight + edit->height() - edit->viewport()->height());
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QWidget w;
   QVBoxLayout layout{&w};
   QLineEdit edit;
   QTextEdit message;
   message.setReadOnly(true);
   message.setText("Foo Bar!");
   layout.addWidget(&edit);
   layout.addWidget(&message);
   layout.addStretch();
   QObject::connect(&edit, &QLineEdit::textChanged, &message, &QTextEdit::setPlainText);
   QObject::connect(message.document()->documentLayout(),
                    &QAbstractTextDocumentLayout::documentSizeChanged,
                    &message, [&]{ updateSize(&message); });
   w.show();
   return app.exec();
}
