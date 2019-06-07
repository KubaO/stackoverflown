// https://github.com/KubaO/stackoverflown/tree/master/questions/c-cpp-library-api-53643120
#include <iostream>
#include <memory>
#include <string>
#include "library.h"

struct Data {
   std::string payload;
   static int counter;
   void print(int value) {
      ++counter;
      std::cout << counter << ": " << value << ", " << payload << std::endl;
   }
};

int Data::counter;

extern "C" void callback1(library_call_type type, int value, void **param) noexcept {
   if (type == LIBRARY_SAMPLE) {
      auto *data = static_cast<Data *>(*param);
      data->print(value);
   }
}

using DataPrintFn = std::function<void(int)>;

extern "C" void callback2(library_call_type type, int value, void **param) noexcept {
   assert(param && *param);
   auto *fun = static_cast<DataPrintFn *>(*param);
   if (type == LIBRARY_SAMPLE)
      (*fun)(value);
   else if (type == LIBRARY_DEREGISTER) {
      delete fun;
      *param = nullptr;
   }
}

void register_callback(Data *data) {
   library_register_callback(&callback1, data);
}

template <typename F>
void register_callback(F &&fun) {
   auto f = std::make_unique<DataPrintFn>(std::forward<F>(fun));
   library_deregister_callback(callback2, f.get());
   library_register_callback(callback2, f.release());
   // the callback will retain the functor
}

int main() {
   Data data;
   data.payload = "payload";

   library_init();
   register_callback(&data);
   register_callback([&](int value) noexcept { data.print(value); });

   library_sample();
   library_sample();
   library_deinit();  // must happen before the 'data' is destructed
   assert(data.counter == 4);
}
