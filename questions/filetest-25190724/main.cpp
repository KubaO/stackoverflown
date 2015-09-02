#include <QElapsedTimer>
#include <QTemporaryFile>
#include <QByteArray>
#include <QFile>
#include <QTextStream>
#include <cstdio>

QTextStream out(stdout);

template <typename F> void time(const char * task, F fun)
{
   QElapsedTimer timer;
   timer.start();
   int const N = 10;
   for (int i = 0; i < N; ++i) fun();
   out << task << " took " << timer.elapsed() << " ms" << endl;
}

QByteArray newData() {
   const int size = 100*1024*1024;
   QByteArray buf(size, Qt::Uninitialized);
   // CoW isn't free, using mem takes ~1/3 less time than using buf[x].
   char * const mem = buf.data();
   for (int j = 0; j <= size - 1; j++)
       mem[j] = j % 250;
   for (int j = size - 1;j >= 0; j--)
       mem[j] = j % 251;
   return buf;
}

void qFileWrite(const QByteArray & data, const char * name,
                QIODevice::OpenMode modeExtras = 0)
{
   QFile f(QString::fromLocal8Bit(name));
   if (! f.open(QIODevice::WriteOnly | modeExtras)) abort();
   if (f.write(data) != data.size()) abort();
}

void cWrite(const QByteArray & data, const char * name)
{
   std::FILE * f = fopen(name, "wb");
   if (!f) abort();
   if (fwrite(data.data(), 1, data.size(), f) != data.size()) abort();
   if (fclose(f) == EOF) abort();
}

QByteArray tempFileName() {
   QTemporaryFile f; f.setAutoRemove(false); if (!f.open()) abort();
   return f.fileName().toLocal8Bit();
}

int main()
{
   time("initialization", newData);
   QByteArray const data = newData();
   time("buffered QFile writes to /dev/null", [&]{
      qFileWrite(data, "/dev/null");
   });
   time("unbuffered QFile writes to /dev/null", [&]{
      qFileWrite(data, "/dev/null", QIODevice::Unbuffered);
   });
   time("fwrites to /dev/null", [&]{
      cWrite(data, "/dev/null");
   });
   time("buffered temporary QFile writes", [&]{
      qFileWrite(data, tempFileName());
   });
   time("unbuffered temporary QFile writes", [&]{
      qFileWrite(data, tempFileName(), QIODevice::Unbuffered);
   });
   time("fwrites to temporary file", [&]{
      cWrite(data, tempFileName());
   });
   return 0;
}
