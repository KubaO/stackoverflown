// https://github.com/KubaO/stackoverflown/tree/master/questions/script-timer-11236970
#include <QtScript>

template <typename T> void addType(QScriptEngine * engine) {
   auto constructor = engine->newFunction([](QScriptContext*, QScriptEngine* engine){
      return engine->newQObject(new T());
   });
   auto value = engine->newQMetaObject(&T::staticMetaObject, constructor);
   engine->globalObject().setProperty(T::staticMetaObject.className(), value);
}

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};

   QScriptEngine engine;
   addType<QTimer>(&engine);
   engine.globalObject().setProperty("qApp", engine.newQObject(&app));

   auto script =
         "var timer = new QTimer(); \n"
         "timer.interval = 1000; \n"
         "timer.singleShot = true; \n"
         "var conn = timer.timeout.connect(function(){ \n"
         "  print(\"timeout\"); \n"
         "  qApp.quit(); \n"
         "}); \n"
         "timer.start();\n";

   engine.evaluate(script);
   return app.exec();
}
