// https://github.com/KubaO/stackoverflown/tree/master/questions/curl-50975613
#include <QtWidgets>
#include <curl/curl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <limits>

class CurlGlobal {
   Q_DISABLE_COPY(CurlGlobal)
   CURLcode rc;
public:
   CurlGlobal() { rc = curl_global_init(CURL_GLOBAL_ALL); }
   ~CurlGlobal() { curl_global_cleanup(); }
   CURLcode code() const { return rc; }
   explicit operator bool() const { return rc == CURLE_OK; }
};

class CurlStringList {
   struct curl_slist *list = {};
public:
   CurlStringList() = default;
   explicit CurlStringList(const QStringList &strings) {
      for (auto &s : strings)
         list = curl_slist_append(list, s.toLatin1().constData());
   }
   CurlStringList &operator=(CurlStringList &&o) {
      std::swap(o.list, list);
      return *this;
   }
   ~CurlStringList() {
      curl_slist_free_all(list);
   }
   struct curl_slist *curl() const { return list; }
};

class CurlEasy : public QObject {
   Q_OBJECT
   friend class CurlMulti;
   CURL *const d = curl_easy_init();
   QAtomicInteger<bool> inMulti = false;
   CURLcode rc = d ? CURLE_OK : (CURLcode)-1;
   char err[CURL_ERROR_SIZE];
   CurlStringList headers;

   bool addToMulti();
   void removeFromMulti();
   static size_t write_callback(const char *ptr, size_t size, size_t nmemb, void *data) {
      Q_ASSERT(data);
      return static_cast<CurlEasy*>(data)->writeCallback(ptr, size * nmemb);
   }
protected:
   virtual qint64 writeCallback(const char *, qint64) {
      return -1;
   }
   void setWriteCallback() {
      curl_easy_setopt(d, CURLOPT_WRITEDATA, (void*)this);
      curl_easy_setopt(d, CURLOPT_WRITEFUNCTION, (void*)write_callback);
   }
   virtual void clear() {}
public:
   CurlEasy(QObject *parent = {}) : QObject(parent)
   {
      curl_easy_setopt(d, CURLOPT_ERRORBUFFER, err);
   }
   ~CurlEasy() override {
      removeFromMulti();
      curl_easy_cleanup(d);
   }
   CURL *c() const { return d; }
   operator CURL*() const { return d; }
   bool ok() const { return rc == CURLE_OK; }
   QString errorString() const { return QString::fromUtf8(err); }
   bool setUrl(const QUrl& url) {
      rc = curl_easy_setopt(d, CURLOPT_URL, url.toEncoded().constData());
      return ok();
   }
   void setMaxRedirects(int count) {
      curl_easy_setopt(d, CURLOPT_FOLLOWLOCATION, count ? 1L : 0L);
      curl_easy_setopt(d, CURLOPT_MAXREDIRS, (long)count);
   }
   void setVerbose(bool v) {
      curl_easy_setopt(d, CURLOPT_VERBOSE, v ? 1L : 0L);
   }
   bool get() {
      if (inMulti.loadAcquire())
         return false;
      clear();
      curl_easy_setopt(d, CURLOPT_HTTPGET, 1L);
      if (addToMulti())
         return true;
      rc = curl_easy_perform(d);
      if (!ok())
         qDebug() << errorString();
      return ok();
   }
   void setHeaders(const QStringList &h) {
      headers = CurlStringList(h);
      curl_easy_setopt(d, CURLOPT_HTTPHEADER, headers.curl());
   }
   void setWriteTo(FILE *);
   void setReadFrom(FILE *);
   void setWriteTo(QIODevice *dev);
   void setReadFrom(QIODevice *dev);
};

namespace detail {
size_t write_iodevice(char *ptr, size_t size, size_t nmemb, void *userdata) {
   if (!userdata)
      return -1;
   auto *dev = static_cast<QIODevice*>(userdata);
   qint64 count = size * nmemb;
   auto written = dev->write(ptr, count);
   if (written == -1)
      return -1;
   if (written == 0 && count)
      return CURL_WRITEFUNC_PAUSE;
   Q_ASSERT(written <= qint64(std::numeric_limits<size_t>::max()));
   return written;
}
size_t read_iodevice(char *ptr, size_t size, size_t nitems, void *instream) {
   if (!instream)
      return CURL_READFUNC_ABORT;
   auto *dev = static_cast<QIODevice*>(instream);
   qint64 count = size * nitems;
   auto read = dev->read(ptr, count);
   if (read == -1)
      return CURL_READFUNC_ABORT;
   Q_ASSERT(read <= qint64(std::numeric_limits<size_t>::max()));
   return read;
}
size_t seek_iodevice(void *userp, curl_off_t offset, int origin) {
   auto *dev = static_cast<QIODevice*>(userp);
   if (!dev || !dev->isOpen())
      return CURL_SEEKFUNC_FAIL;
   if (origin == SEEK_SET) {
      if (dev->isSequential())
         return CURL_SEEKFUNC_CANTSEEK;
      if (!dev->seek(offset))
         return CURL_SEEKFUNC_FAIL;
   }
   else if (origin == SEEK_CUR) {
      if (offset > 0) {
         auto rc = dev->skip(offset);
         if (rc == -1)
            return CURL_SEEKFUNC_FAIL;
         if (rc != offset)
            return CURL_SEEKFUNC_CANTSEEK;
      } else if (offset < 0) {
         if (dev->isSequential())
            return CURL_SEEKFUNC_CANTSEEK;
         if (!dev->seek(dev->pos() + offset))
            return CURL_SEEKFUNC_FAIL;
      }
   }
   else if (origin == SEEK_END) {
      if (dev->isSequential())
         return CURL_SEEKFUNC_CANTSEEK;
      if (offset > 0 || !dev->seek(dev->size() + offset))
         return CURL_SEEKFUNC_FAIL;
   }
   else
      return CURL_SEEKFUNC_CANTSEEK;
   return CURL_SEEKFUNC_OK;
}
int seek_file(void *userp, curl_off_t offset, int origin) {
   auto *file = static_cast<FILE*>(userp);
   auto rc = fseek(file, offset, origin);
   return (rc == 0) ? CURL_SEEKFUNC_OK : CURL_SEEKFUNC_FAIL;
}
}

#ifdef _WIN32
using stat = __stat64;
int fstat(int fd, stat *buffer) { return _fstat64(fd, buffer); }
int fileno(FILE *f) { return _fileno(f); }
#endif

void CurlEasy::setWriteTo(QIODevice *dev) {
   Q_ASSERT(dev && dev->isWritable());
   curl_easy_setopt(d, CURLOPT_WRITEDATA, (void*)dev);
   curl_easy_setopt(d, CURLOPT_WRITEFUNCTION, (void*)detail::write_iodevice);
}

void CurlEasy::setReadFrom(QIODevice *dev) {
   Q_ASSERT(dev && dev->isReadable());
   curl_easy_setopt(d, CURLOPT_READDATA, (void*)dev);
   curl_easy_setopt(d, CURLOPT_READFUNCTION, (void*)detail::read_iodevice);
   curl_easy_setopt(d, CURLOPT_SEEKDATA, (void*)dev);
   curl_easy_setopt(d, CURLOPT_SEEKFUNCTION, (void*)detail::seek_iodevice);
   curl_off_t size = dev->isSequential() ? -1 : dev->size();
   curl_easy_setopt(d, CURLOPT_INFILESIZE_LARGE, size);
}

void CurlEasy::setWriteTo(FILE *file) {
   Q_ASSERT(file);
   curl_easy_setopt(d, CURLOPT_WRITEDATA, (void*)file);
   curl_easy_setopt(d, CURLOPT_WRITEFUNCTION, (void*)nullptr);
   curl_easy_setopt(d, CURLOPT_SEEKDATA, (void*)file);
   curl_easy_setopt(d, CURLOPT_SEEKFUNCTION, (void*)detail::seek_file);
}

void CurlEasy::setReadFrom(FILE *file) {
   Q_ASSERT(file);
   curl_easy_setopt(d, CURLOPT_READDATA, (void*)file);
   curl_easy_setopt(d, CURLOPT_READFUNCTION, (void*)nullptr);
   curl_easy_setopt(d, CURLOPT_SEEKDATA, (void*)file);
   curl_easy_setopt(d, CURLOPT_SEEKFUNCTION, (void*)detail::seek_file);
   curl_off_t size = -1;
   struct stat buf;
   if (fstat(fileno(file), &buf) == 0)
      size = buf.st_size;
   curl_easy_setopt(d, CURLOPT_INFILESIZE_LARGE, size);
}

class CurlLineParser : public CurlEasy {
   Q_OBJECT
   QByteArray m_buf;
protected:
   qint64 writeCallback(const char *ptr, qint64 count) override {
      const char *start = ptr;
      const char *const end = ptr + count;
      for (; ptr != end; ptr++) {
         if (*ptr == '\n') {
            if (!m_buf.isEmpty())
               m_buf.append(start, ptr-start);
            else
               m_buf = QByteArray::fromRawData(start, ptr-start);
            emit hasLine(QString::fromUtf8(m_buf));
            m_buf.clear();
            start = ptr + 1;
         }
      }
      // keep partial line
      m_buf = QByteArray::fromRawData(start, ptr-start);
      return count;
   }
   void clear() override {
      m_buf.clear();
   }
public:
   CurlLineParser(QObject *parent = {}) : CurlEasy(parent) {
      setWriteCallback();
   }
   Q_SIGNAL void hasLine(const QString &);
};

class CurlMulti : public QObject {
   Q_OBJECT
   friend class CurlEasy;
   struct Notifiers {
      QScopedPointer<QSocketNotifier> read, write, exception;
   };
   CURLM *m = curl_multi_init();
   QBasicTimer m_timer;

   void timerEvent(QTimerEvent *ev) override {
      int running;
      if (ev->timerId() == m_timer.timerId()) {
         m_timer.stop();
         curl_multi_socket_action(m, CURL_SOCKET_TIMEOUT, 0, &running);
         processInfo();
      }
   }
   static int socket_callback(CURL *, curl_socket_t s, int what, void *userp, void *socketp) {
      Q_ASSERT(userp);
      return static_cast<CurlMulti*>(userp)->
            socketCallback(s, what, static_cast<Notifiers*>(socketp));
   }
   static int timer_callback(CURLM *, long ms, void *userp) {
      Q_ASSERT(userp);
      auto *q = static_cast<CurlMulti*>(userp);
      if (ms == -1)
         q->m_timer.stop();
      else if (ms >= 0)
         q->m_timer.start(ms, q);
      return 0;
   }
   int socketCallback(curl_socket_t s, int what, Notifiers* n) {
      if (what == CURL_POLL_REMOVE) {
         delete n;
         curl_multi_assign(m, s, nullptr);
         processInfo();
         return 0;
      }
      if (!n) {
         n = new Notifiers;
         curl_multi_assign(m, s, n);
         n->exception.reset(new QSocketNotifier(s, QSocketNotifier::Exception, this));
         connect(&*n->exception, &QSocketNotifier::activated, [this, s]{
            int running;
            curl_multi_socket_action(m, s, CURL_CSELECT_ERR, &running);
         });
      }
      if ((what & CURL_POLL_IN) && !n->read) {
         n->read.reset(new QSocketNotifier(s, QSocketNotifier::Read, this));
         connect(&*n->read, &QSocketNotifier::activated, [this, s]{
            int running;
            curl_multi_socket_action(m, s, CURL_CSELECT_IN, &running);
         });
      }
      if ((what & CURL_POLL_OUT) && !n->write) {
         n->write.reset(new QSocketNotifier(s, QSocketNotifier::Write, this));
         connect(&*n->write, &QSocketNotifier::activated, [this, s]{
            int running;
            curl_multi_socket_action(m, s, CURL_CSELECT_OUT, &running);
         });
      }
      n->exception->setEnabled(what & CURL_POLL_INOUT);
      if (n->read)
         n->read->setEnabled(what & CURL_POLL_IN);
      if (n->write)
         n->write->setEnabled(what & CURL_POLL_OUT);
      processInfo();
      return 0;
   }
   void processInfo() {
      int msgq;
      while (const auto *info = curl_multi_info_read(m, &msgq)) {
         if (info->msg == CURLMSG_DONE)
            for (auto *c : children())
               if (auto *b = qobject_cast<CurlEasy*>(c))
                  if (b->d == info->easy_handle && b->inMulti)
                     b->removeFromMulti();
      }
   }
public:
   CurlMulti(QObject *parent = {}) : QObject(parent) {
      curl_multi_setopt(m, CURLMOPT_SOCKETDATA, (void*)this);
      curl_multi_setopt(m, CURLMOPT_SOCKETFUNCTION, (void*)socket_callback);
      curl_multi_setopt(m, CURLMOPT_TIMERDATA, (void*)this);
      curl_multi_setopt(m, CURLMOPT_TIMERFUNCTION, (void*)timer_callback);
      int running;
      curl_multi_socket_action(m, CURL_SOCKET_TIMEOUT, 0, &running);
   }
   Q_SIGNAL void finished(CurlEasy *);
   ~CurlMulti() override {
      for (auto *c : children())
         if (auto *b = qobject_cast<CurlEasy*>(c))
            b->removeFromMulti();
      curl_multi_cleanup(m);
   }
};

bool CurlEasy::addToMulti() {
   auto *m = qobject_cast<CurlMulti*>(parent());
   if (d && m && inMulti.testAndSetOrdered(false, true)) {
      QMetaObject::invokeMethod(m, [m, this]{
         curl_multi_add_handle(m->m, d);
      });
   }
   return inMulti;
}

void CurlEasy::removeFromMulti() {
   auto *m = qobject_cast<CurlMulti*>(parent());
   if (d && m && inMulti.testAndSetOrdered(true, false)) {
      curl_multi_remove_handle(m->m, d);
      emit m->finished(this);
   }
}

class Ui : public QWidget {
   Q_OBJECT
   CurlLineParser m_curl{this};
   QRegularExpression m_re{"[a-z0-9]+[_a-z0-9.-]*[a-z0-9]+@[a-z0-9-]+(.[a-z0-9-]+)"};
   QStringList m_emails;
   QGridLayout m_layout{this};
   QPlainTextEdit m_view;
   QLineEdit m_url;
   QCheckBox m_async{"Async"};
   QPushButton m_get{"Get"};
public:
   Ui() {
      m_url.setPlaceholderText("Url");
      m_view.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
      m_url.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
      m_layout.addWidget(&m_view, 0, 0, 1, 3);
      m_layout.addWidget(&m_url, 1, 0);
      m_layout.addWidget(&m_async, 1, 1);
      m_layout.addWidget(&m_get, 1, 2);
      connect(&m_get, &QPushButton::clicked, this, [this]{
         m_emails.clear();
         emit requestGet(m_url.text());
      });
      connect(&m_async, &QCheckBox::toggled, this, &Ui::requestAsync);
   }
   Q_SIGNAL void requestGet(const QString &);
   Q_SIGNAL void requestAsync(bool);
   void setUrl(const QString &url) { m_url.setText(url); }
   void setAsync(bool async) { m_async.setChecked(async); }
   void processLine(const QString &line) {
      auto it = m_re.globalMatch(line);
      while (it.hasNext()) {
         auto email = it.next().captured(0);
         m_emails.push_back(email);
         m_view.appendPlainText(email);
      }
   }
};

int main(int argc, char* argv[]) {
   CurlGlobal curlGlobal;
   QApplication app(argc, argv);
   QThread netThread;
   CurlMulti multi;
   CurlLineParser curl;
   multi.moveToThread(&netThread);
   Ui ui;
   QObject::connect(&ui, &Ui::requestGet, [&](const QString &url){
      curl.setUrl(url);
      curl.get();
   });
   QObject::connect(&ui, &Ui::requestAsync, [&](bool async){
      QMetaObject::invokeMethod(&curl, [&curl, &multi, async]{
         if (async) curl.moveToThread(multi.thread());
         curl.setParent(async ? &multi : nullptr);
         if (!async) curl.moveToThread(qApp->thread());
      });
   });
   QObject::connect(&curl, &CurlLineParser::hasLine, &ui, &Ui::processLine);
   curl.setMaxRedirects(2);
   curl.setVerbose(false);
   curl.setHeaders({"Accept: text/plain; charset=utf-8"});
   ui.setUrl("https://gist.github.com/retronym/f9419787089149ad3a59835c2e1ab81a/"
             "raw/8cd88ab3645508ae1243f3aa4ec7c012af4fae7b/emails.txt");
   ui.show();
   netThread.start();
   int rc = app.exec();
   QMetaObject::invokeMethod(&multi, [&]{
      if (!curl.parent())
         curl.moveToThread(app.thread());
      multi.moveToThread(app.thread());
      netThread.quit();
   });
   netThread.wait();
   return rc;
}

#include "main.moc"

// See also:
// https://github.com/tarasvb/qtcurl
// https://github.com/pavlonion/qtcurl
