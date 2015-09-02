#include <QFile>
#include <QDir>
#include <list>

int main()
{
   std::list<QFile> files;
   files.emplace_back(QDir::homePath() + QDir::separator() + "test.txt");
   files.front().open(QIODevice::WriteOnly);
   files.front().write("abcdef\n");
}
