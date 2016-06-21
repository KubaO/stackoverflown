// https://github.com/KubaO/stackoverflown/tree/master/questions/qreal-literal-37876934
#include <QtCore>

constexpr qreal operator "" _qr(long double a){ return qreal(a); }

int main1() {
   qreal a = 3.0_qr;
   Q_ASSERT(qMin(a, 4.0_qr) == a);
}

using _qr  = qreal;

int main() {
   qreal a = _qr(3.0);
   Q_ASSERT(qMin(a, _qr(4.0)) == a);
}



