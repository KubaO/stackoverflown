// https://github.com/KubaO/stackoverflown/tree/master/questions/textedit-columns-37949301
#include <QtWidgets>

template <typename It, typename F>
QString toTable(It begin, It end, int columns, F && format,
                const QString & attributes = QString()) {
   if (begin == end) return QString();
   QString output = QStringLiteral("<table %1>").arg(attributes);
   int n = 0;
   for (; begin != end; ++begin) {
      if (!n) output += "<tr>";
      output += "<td>" + format(*begin) + "</td>";
      if (++n == columns) {
         n = 0;
         output += "</tr>";
      }
   }
   output += "</table>";
   return output;
}

#include <QtConcurrent>

void setHtml(QTextEdit * edit, const QString & html) {
   QtConcurrent::run([=]{
      // runs in a worker thread
      auto doc = new QTextDocument;
      QObject src;
      src.connect(&src, &QObject::destroyed, edit, [=]{
         // runs in the main thread
         doc->setParent(edit);
         edit->setDocument(doc);
      });
      doc->setHtml(html);
      doc->moveToThread(edit->thread());
   });
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   double const table[] {
      78.0,    78.0,    78.0,
      0.0,     0.0,     78.0,
      69.0,    56.0,    0.0};
   QTextEdit edit;
   setHtml(&edit, toTable(std::begin(table), std::end(table), 3,
                          [](double arg){ return QString::number(arg, 'f', 1); },
   "width=\"100%\""));
   edit.setReadOnly(true);
   edit.show();
   return app.exec();
}
