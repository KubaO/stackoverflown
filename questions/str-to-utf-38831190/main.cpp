// https://github.com/KubaO/stackoverflown/tree/master/questions/str-to-utf-38831190
#include <QtCore>

QByteArray toUtf8Hex(const QString & str) {
   return str.toUtf8().toHex();
}
QString fromUtf8Hex(const QByteArray & hex) {
   return QString::fromUtf8(QByteArray::fromHex(hex));
}

QByteArray toUtf16Hex(QString str) {
   str.prepend(QChar::ByteOrderMark);
   // It is OK to use `fromRawData` since toHex copies it.
   return QByteArray::fromRawData(
            reinterpret_cast<const char*>(str.constData()), (str.size()+1)*2).toHex();
}
QString fromUtf16Hex(const QByteArray & hex) {
   const QByteArray utf16 = QByteArray::fromHex(hex);
   return QString::fromUtf16(reinterpret_cast<const quint16*>(utf16.data()));
}

int main() {
   const QString str = QStringLiteral("H€w ar€ you?");

   // To Utf8 and back
   const QByteArray hex8 = toUtf8Hex(str);
   Q_ASSERT(fromUtf8Hex(hex8) == str);

   // To Utf16 and back
   const QByteArray hex16 = toUtf16Hex(str);
   Q_ASSERT(fromUtf16Hex(hex16) == str);
}
