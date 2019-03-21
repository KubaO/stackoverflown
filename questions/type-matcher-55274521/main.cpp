#include <string>
#include <vector>
#include <map>
#include <optional>

/* Grammar: https://www.cs.dartmouth.edu/~mckeeman/cs48/references/c.html
Here, we implement an abbreviated variant:

type
  declaration-specifier declarator;
declaration-specifier
  type-qualifier? type-specifier
type-specifier
  "int"
  "bool"
type-qualifier
  "const"
declarator
  pointer? direct-declarator
pointer
  "*" type-qualifier? pointer?
direct-declarator
  identifier
  identifier "[" int-literal? "]"

*/






int main() {
   struct Problem {
      std::string parameter;
      std::string argument;
   };
   std::vector<Problem> problem{{"T1", ""}};

   std::vector<std::string> templ_params{"T1"};
   std::vector<std::string> templ_args(templ_params.size());
   std::string str_param = "T1*";
   std::string str_arg = "int**";
   const char *p_str_param = str_param.c_str();
   const char *p_str_arg = str_arg.c_str();
   while (*p_str_param != 0 && *p_str_arg != 0) {
      while (*p_str_param == ' ' || *p_str_param == '\t') p_str_param++;
      while (*p_str_arg == ' ' || *p_str_arg == '\t') p_str_arg++;
      if (*p_str_param == 0 || *p_str_arg == 0) break;
      if (*p_str_param != *p_str_arg) {
         std::string templ_param;
         std::string templ_arg;
         while (*p_str_param == '_' || (*p_str_param >= 'a' && *p_str_param <= 'z') ||
                (*p_str_param >= 'A' && *p_str_param <= 'Z') ||
                (*p_str_param >= '0' && *p_str_param <= '9'))
            templ_param += *(p_str_param++);
         while (*p_str_param == ' ' || *p_str_param == '\t') p_str_param++;
         char end_marker = *p_str_param;
         const char *p_str_arg_end = p_str_arg;
         while (*p_str_arg_end != end_marker) p_str_arg_end++;
         while (*(p_str_arg_end - 1) == ' ' || *(p_str_arg_end - 1) == '\t')
            p_str_arg_end--;
         while (p_str_arg < p_str_arg_end) templ_arg += *(p_str_arg++);
         for (size_t i = 0; i < templ_params.size(); j++) {
            if (templ_params[i] == templ_param) {
               templ_args[i] = templ_arg;
               break;
            }
         }
      } else {
         p_str_param++;
         p_str_arg++;
      }
   }
}
