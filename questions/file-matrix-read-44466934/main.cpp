// https://github.com/KubaO/stackoverflown/tree/master/questions/file-matrix-read-44466934
#include <QtCore>
#include <cstdlib>

using Matrix = QVector<QVector<double>>;

// A valid matrix has a positive row size, and consists of a non-zero number of
// rows all having the same number of elements equal to the row size.
// Whitespace separates the elements.
// The matrix will be left unchanged if it doesn't conform to spec above, or if
// an I/O error had occurred. Non-conforming matrices set a ReadCorruptData
// status.
QTextStream & operator>>(QTextStream & in, Matrix & out) {
   Matrix matrix;
   int columns;
   in >> columns;
   if (in.status() != QTextStream::Ok)
      return in;
   if (columns <= 0) {
      in.setStatus(QTextStream::ReadCorruptData);
      return in;
   }
   for (int col = 0; !in.atEnd(); col++) {
      double element;
      in >> element;
      if (in.status() == QTextStream::ReadPastEnd)
         break;
      if (in.status() != QTextStream::Ok)
         return in;
      if (col >= columns)
         col = 0;
      if (col == 0) {
         matrix.push_back({});
         matrix.back().reserve(columns);
      }
      matrix.back().push_back(element);
   }
   if (!matrix.isEmpty() && matrix.back().size() == columns)
      out = matrix;
   else
      in.setStatus(QTextStream::ReadCorruptData);
   return in;
}

const char
rawData[] = "5\n"
            "1 2 3 4 5\n"
            "5 6 7 8 9\n"
            "10 11 12 13 14\n"
            "15 16 17 18 19\n"
            "20 21 22 23 24";

int main()
{
   auto data = QByteArray::fromRawData(rawData, sizeof(rawData)-1);
   QBuffer file(&data);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      return qCritical("Input file can't be opened."), EXIT_FAILURE;
   QTextStream in(&file);
   Matrix matrix;
   in >> matrix;
   qDebug() << matrix;
   Matrix compare{{1,2,3,4,5}, {5,6,7,8,9}, {10,11,12,13,14}, {15,16,17,18,19}, {20,21,22,23,24}};
   Q_ASSERT(matrix == compare);
}
