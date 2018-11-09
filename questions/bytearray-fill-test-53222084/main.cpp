// https://github.com/KubaO/stackoverflown/tree/master/questions/bytearray-fill-test-53222084
#include <QtCore>
#include <algorithm>

int main() {
   constexpr int N = 100;

   QByteArray array1(N, '\0');
   Q_ASSERT(array1.size() == N);
   Q_ASSERT(
       std::all_of(array1.cbegin(), array1.cend(), [](QChar c) { return c == '\0'; }));

   QByteArray array2;
   array2.reserve(N * 3 - 1);
   for (int i = N - 1; i >= 0; --i) {
      array2.append(i ? "00 " : "00");
   }
   Q_ASSERT(array2.size() == (N * 3) - 1);
   Q_ASSERT(std::all_of(array2.cbegin(), array2.cend(), [i = 0](QChar c) mutable {
      return c == (i++ % 3 == 2 ? ' ' : '0');
   }));
}
