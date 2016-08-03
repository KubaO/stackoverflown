// https://github.com/KubaO/stackoverflown/tree/master/questions/textdoc-iter-38732207
#include <QtWidgets>

void iterate(QTextEdit * edit) {
   auto const & doc = *edit->document();
   for (auto block = doc.begin(); block != doc.end(); block.next()) {
      // do something with text block e.g. iterate its fragments
      for (auto fragment = block.begin(); fragment != block.end(); fragment++) {
         // do something with text fragment
      }
   }
}

int main() {}
