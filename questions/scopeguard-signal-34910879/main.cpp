// https://github.com/KubaO/stackoverflown/tree/master/questions/scopeguard-signal-34910879
// main.cpp
#include <QtCore>

class ScopeSignaller : public QObject {
   Q_OBJECT
public:
   Q_SIGNAL void inScope(bool);
   template <typename F>
   ScopeSignaller(QObject * target, F && slot, QObject * parent = 0) : QObject(parent) {
      connect(this, &ScopeSignaller::inScope, target, std::forward<F>(slot));
      inScope(true);
   }
   ~ScopeSignaller() {
      inScope(false);
   }
};

int main(int argc, char ** argv) {
   QCoreApplication app{argc, argv};
   ScopeSignaller s(&app, +[](bool b){ qDebug() << "signalled" << b; });
}

#include "main.moc"
