#include <QString>
#include <QDebug>
#include <QRegExp>

#if 0
int main()
{
   QString str1(QStringLiteral("A"));
   QString str2("A");
   const QChar * p = str1.constData();
   while (p->unicode()) qDebug() << *p++;
   p = str2.constData();
   while (p->unicode()) qDebug() << *p++;
}
#endif

#if 0
int main()
{
   uint data[] = { 0x10398, 0 };
   QString s = QString::fromUcs4(data);
   QRegExp r("^([\\xD800][\\xDF98])$");
   qDebug() << s.size() << s.contains(r);
}
#endif


#if 1
int main()
{
   uint data[] = { 0x30, 0x40, 0x10000, 0x1007F, 0 };
   QString s = QString::fromUcs4(data);
   QRegExp r("^([\\x0-\\xD7FF\\xE000-\\xFFFF]|([\\xD800][\\xDC00-\\xDC7F]))+$");
   qDebug() << s.size() << s.contains(r);
}
#endif
