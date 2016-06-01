// https://github.com/KubaO/stackoverflown/tree/master/questions/dupechecker-37557870
#include <QtCore>

class Checker {
public:
   void check(const QString & path) {
      QDir dir(path);

   }
};

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   QString dir;
   if (argc == 2)
      dir = app.arguments().at(1);
   Checker checker;
   checker.check(dir);
}

