#include <QtGlobal>

class Object {};
class Interface {
public:
   static Object * GetUI();
};

// This must be in a *single* source file *only*. Not in header!
Q_GLOBAL_STATIC(Object, interfaceInstance)

Object* Interface::GetUI() {
    return interfaceInstance;
}

int main()
{
   return 0;
}
