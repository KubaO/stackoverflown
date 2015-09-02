#include <QString>
#include <QTextStream>
#include <cstdio>
#include <string>

int main(int argc, char *argv[])
{
   QTextStream out(stdout);
   std::string str = "Example \"how\" \"are\" \"you\""; //str contains: Example "how" "are" "you"
   QString qstr(str.c_str()); //qstr contains: Example how are you
   out << str.c_str() << endl << qstr << endl;
   qstr = nullptr;
   out << qstr << endl;
   return 0;
}
