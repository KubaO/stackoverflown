// https://github.com/KubaO/stackoverflown/tree/master/questions/keyname-21764138
#include <QMetaEnum>

namespace SO {
enum KeyNameOption { KeyNameNone = 0, AppendArrow = 1 };
Q_DECLARE_FLAGS(KeyNameOptions, KeyNameOption)
}
QString keyName(int index, SO::KeyNameOptions opt = {}) {
   constexpr static auto const getEnum = [](const char *name) {
      int enumIndex = qt_getQtMetaObject()->indexOfEnumerator(name);
      return qt_getQtMetaObject()->enumerator(enumIndex);
   };
   static const auto keyEnum = getEnum("Key");
   static const auto modifierEnum = getEnum("KeyboardModifiers");

   auto name = modifierEnum.valueToKeys(index & Qt::KeyboardModifierMask);
   index &= ~Qt::KeyboardModifierMask;

   if (name == "NoModifier")
      name.clear();
   else {
      name.replace('|', '+');
      name.replace("Modifier", "");
      name.append('+');
   }

   auto keyName = keyEnum.valueToKey(index);
   if (keyName)
      name.append(keyName + 4);
   if ((opt & SO::AppendArrow) && index >= Qt::Key_Left && index <= Qt::Key_Down)
      name.append(" Arrow");
   return QLatin1String(name);
}

int main() {
   Q_ASSERT(keyName(Qt::Key_Tab) == "Tab");
   Q_ASSERT(keyName(Qt::ShiftModifier | Qt::Key_Up, SO::AppendArrow) == "Shift+Up Arrow");
   Q_ASSERT(keyName(Qt::AltModifier | Qt::Key_Down) == "Alt+Down");
}
