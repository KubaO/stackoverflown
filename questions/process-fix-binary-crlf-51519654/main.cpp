// https://github.com/KubaO/stackoverflown/tree/master/questions/process-fix-binary-crlf-51519654
#include <QtCore>
#include <algorithm>

bool appendBinFix(QByteArray &buf, const char *src, int size) {
   bool okData = true;
   if (!size) return okData;
   constexpr char CR = '\x0d';
   constexpr char LF = '\x0a';
   bool hasCR = buf.endsWith(CR);
   buf.resize(buf.size() + size);
   char *dst = buf.end() - size;
   const char *lastSrc = src;
   for (const char *const end = src + size; src != end; src++) {
      char const c = *src;
      if (c == LF) {
         if (hasCR) {
            std::copy(lastSrc, src, dst);
            dst += (src - lastSrc);
            dst[-1] = LF;
            lastSrc = src + 1;
         } else
            okData = false;
      }
      hasCR = (c == CR);
   }
   dst = std::copy(lastSrc, src, dst);
   buf.resize(dst - buf.constData());
   return okData;
}

bool appendBinFix(QByteArray &buf, const QByteArray &src) {
   return appendBinFix(buf, src.data(), src.size());
}

#include <QtTest>
#include <cstdio>
#ifdef Q_OS_WIN
#include <fcntl.h>
#include <io.h>
#endif

const auto dataFixed = QByteArrayLiteral("\x00\x11\x0d\x0a\x33");
const auto data = QByteArrayLiteral("\x00\x11\x0d\x0d\x0a\x33");

int writeOutput() {
#ifdef Q_OS_WIN
   _setmode(_fileno(stdout), _O_BINARY);
#endif
   auto size = fwrite(data.data(), 1, data.size(), stdout);
   qDebug() << size << data.size();
   return (size == data.size()) ? 0 : 1;
}

class AppendTest : public QObject {
   Q_OBJECT
   struct Result {
      QByteArray d;
      bool ok;
      bool operator==(const Result &o) const { return ok == o.ok && d == o.d; }
   };
   static Result getFixed(const QByteArray &src, int split) {
      Result f;
      f.ok = appendBinFix(f.d, src.data(), split);
      f.ok = appendBinFix(f.d, src.data() + split, src.size() - split) && f.ok;
      return f;
   }
   Q_SLOT void worksWithLFCR() {
      const auto lf_cr = QByteArrayLiteral("\x00\x11\x0a\x0d\x33");
      for (int i = 0; i < lf_cr.size(); ++i)
         QCOMPARE(getFixed(lf_cr, i), (Result{lf_cr, false}));
   }
   Q_SLOT void worksWithCRLF() {
      const auto cr_lf = QByteArrayLiteral("\x00\x11\x0d\x0a\x33");
      const auto cr_lf_fixed = QByteArrayLiteral("\x00\x11\x0a\x33");
      for (int i = 0; i < cr_lf.size(); ++i)
         QCOMPARE(getFixed(cr_lf, i), (Result{cr_lf_fixed, true}));
   }
   Q_SLOT void worksWithCRCRLF() {
      for (int i = 0; i < data.size(); ++i) QCOMPARE(getFixed(data, i).d, dataFixed);
   }
   Q_SLOT void worksWithQProcess() {
      QProcess proc;
      proc.start(QCoreApplication::applicationFilePath(), {"output"},
                 QIODevice::ReadOnly);
      proc.waitForFinished(5000);
      QCOMPARE(proc.exitCode(), 0);
      QCOMPARE(proc.exitStatus(), QProcess::NormalExit);

      QByteArray out = proc.readAllStandardOutput();
      QByteArray fixed;
      appendBinFix(fixed, out);
      QCOMPARE(out, data);
      QCOMPARE(fixed, dataFixed);
   }
};

int main(int argc, char *argv[]) {
   QCoreApplication app(argc, argv);
   if (app.arguments().size() > 1) return writeOutput();
   AppendTest test;
   QTEST_SET_MAIN_SOURCE_PATH
   return QTest::qExec(&test, argc, argv);
}
#include "main.moc"
