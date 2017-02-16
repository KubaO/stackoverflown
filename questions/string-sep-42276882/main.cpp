// https://github.com/KubaO/stackoverflown/tree/master/questions/string-sep-42276882
#include <QtCore>

QString separate1(const QString & string, const QString & separator) {
   QString result;
   result.reserve(string.size() * (1 + separator.size()));
   for (auto ch : string) {
      result.append(ch);
      result.append(separator);
   }
   result.chop(separator.size());
   return result;
}

QString separate(const QString & string, const QString & separator) {
   QString result{string.size() + (string.size()-1) * separator.size(),
                  Qt::Uninitialized};
   auto const end = result.data() + result.size();
   int s{};
   for (auto p = result.data(); p < end;) {
     *p++ = string.at(s++);
      if (Q_LIKELY(p < end))
         for (auto const ch : separator)
            *p++ = ch;
   }
   return result;
}

int main() {
   auto const separator = QStringLiteral("-");
   auto const source = QStringLiteral("Hello world!");
   auto const compare = QStringLiteral("H-e-l-l-o- -w-o-r-l-d-!");
   Q_ASSERT(separate1(source, separator) == compare);
   Q_ASSERT(separate(source, separator) == compare);
}
