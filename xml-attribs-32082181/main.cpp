#include <QByteArray>
#include <QDomDocument>
#include <QDebug>

int main()
{
   QDomDocument doc;
   QByteArray bytes("<special-prop key=\"A\">1</special-prop><special-prop key=\"B\">2</special-prop><special-prop key=\"C\">3</special-prop>");
   doc.setContent(bytes);

   auto start = doc.elementsByTagName("special-prop");
   auto node = start.item(0);
   qDebug() << "Element text: " << node.toElement().text();

   auto map = node.attributes();
   qDebug() << "Attributes found: " << map.length();

   for (int i = 0; i < map.length(); ++i) {
      auto inode = map.item(i);
      auto attr = inode.toAttr();
      qDebug() << "Attribute: " << attr.name() << "= " << attr.value();
   }
   return 0;
}
