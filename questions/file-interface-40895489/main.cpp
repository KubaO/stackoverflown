// https://github.com/KubaO/stackoverflown/tree/master/questions/file-interface-40895489
#include <cstdint>

class InputInterface {
protected:
   InputInterface() {}
public:
   virtual int64_t read(char *, int64_t maxSize) = 0;
   virtual int64_t pos() const = 0;
   virtual bool seek(int64_t) = 0;
   virtual bool isOpen() const = 0;
   virtual bool atEnd() const = 0;
   virtual bool ok() const = 0;
   virtual bool flush() = 0;
   virtual void close() = 0;
   virtual ~InputInterface() {}
};

#include <QtCore>

class QtFile : public InputInterface {
   QFile f;
public:
   QtFile() {}
   QtFile(const QString &name) : f(name) {}
   bool open(const QString &name, QFile::OpenMode mode) {
      close();
      f.setFileName(name);
      return f.open(mode);
   }
   bool open(QFile::OpenMode mode) {
      close();
      return f.open(mode);
   }
   void close() override {
      f.close();
   }
   bool flush() override {
      return f.flush();
   }
   int64_t read(char * buf, int64_t maxSize) override {
      return f.read(buf, maxSize);
   }
   int64_t pos() const override {
      return f.pos();
   }
   bool seek(int64_t pos) override {
      return f.seek(pos);
   }
   bool isOpen() const override {
      return f.isOpen();
   }
   bool atEnd() const override {
      return f.atEnd();
   }
   bool ok() const override {
      return f.isOpen() && f.error() == QFile::NoError;
   }
   QString statusString() const {
      return f.errorString();
   }
};

#include <cstdio>
#include <cerrno>
#include <cassert>
#include <string>

class CFile : public InputInterface {
   FILE *f = nullptr;
   int mutable m_status = 0;
public:
   CFile() {}
   CFile(FILE * f) : f(f) {
      assert(!ferror(f)); // it is impossible to retrieve the error at this point
      m_status = 0;
   }
   ~CFile() { close(); }
   void close() override {
      if (f) fclose(f);
      f = nullptr;
      m_status = 0;
   }
   bool open(const char *name, const char *mode) {
      close();
      f = fopen(name, mode);
      if (!f) m_status = errno;
      return f;
   }
   bool flush() override {
      auto rc = fflush(f);
      if (rc) m_status = errno;
      return !rc;
   }
   bool isOpen() const override { return f; }
   bool atEnd() const override { return f && feof(f); }
   bool ok() const override { return f && !m_status; }
   int64_t read(char * buf, int64_t maxSize) override {
      auto n = fread(buf, 1, maxSize, f);
      if (ferror(f)) m_status = errno;
      return n;
   }
   bool seek(int64_t pos) override {
      auto rc = fseek(f, pos, SEEK_SET);
      if (rc) m_status = errno;
      return !rc;
   }
   int64_t pos() const override {
      if (!f) return 0;
      auto p = ftell(f);
      if (p == EOF) {
         m_status = errno;
         return 0;
      }
      return p;
   }
   std::string statusString() const {
      return {strerror(m_status)};
   }
};

int main() {
   CFile cf;
   QtFile qf;
}
