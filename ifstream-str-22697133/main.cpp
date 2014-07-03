#include <fstream>
#include <string>

int main()
{
   for (int i = 0; i < 1000; ++ i) {
      std::string path("foo bar baz");
      std::ifstream cmdlineStream(path.c_str(),std::ios_base::in);
      if (cmdlineStream.is_open()){
         std::string name;
         std::getline(cmdlineStream, name,'\0');
      }
   }
   return 0;
}
