#include <QMetaEnum>

class KeyHelper : private QObject {
public:
   static QString keyName(int index) {
      static int keyEnumIndex = staticQtMetaObject.indexOfEnumerator("Key");
      QString name = staticQtMetaObject.enumerator(keyEnumIndex).valueToKey(index);
      if (index >= Qt::Key_Left && index <= Qt::Key_Down) name += " Arrow";
      return name.isEmpty() ? QString() : name.mid(4);
   }
};

int main()
{
   Q_ASSERT(KeyHelper::keyName(Qt::Key_Tab) == "Tab");
   Q_ASSERT(KeyHelper::keyName(Qt::Key_Up) == "Up Arrow");
}
