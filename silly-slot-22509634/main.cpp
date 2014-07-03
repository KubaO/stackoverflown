#include <vector>
#include <QObject>
#include <QCoreApplication>
#include <QAtomicInt>

static QAtomicInt n = 0;
class Object : public QObject {
   Q_OBJECT
public:
   Q_SIGNAL void source(const std::vector<uint8_t> &);
   Q_SLOT void sink(std::vector<uint8_t> data) {
      if (data.size() > 20) data.resize(data.size() - 20);
      n.fetchAndAddOrdered(1);
   }
};
Q_DECLARE_METATYPE(std::vector<uint8_t>)

int main(int argc, char ** argv)
{
   QCoreApplication a(argc, argv);
   qRegisterMetaType<std::vector<uint8_t> >();
   Object src, dstD, dstQ;
   const int N = 1000000;
   // note elision of const & from the parameter types
   dstD.connect(&src, SIGNAL(source(std::vector<uint8_t>)),
               SLOT(sink(std::vector<uint8_t>)));
   dstQ.connect(&src, SIGNAL(source(std::vector<uint8_t>)),
               SLOT(sink(std::vector<uint8_t>)), Qt::QueuedConnection);
   for (int i = 0; i < N; ++i) {
      std::vector<uint8_t> v;
      v.resize(qrand() % 100);
      emit src.source(v);
   }
   a.processEvents();
   Q_ASSERT(n.loadAcquire() == (2*N));
   return 0;
}

#include "main.moc"
