#include <QApplication>
#include <QByteArray>
#include <QProgressDialog>
#include <QFileDialog>
#include <QBasicTimer>
#include <QElapsedTimer>
#include <QThread>
#include <QMutex>
#include <QFile>
#include <limits>

class FileCopier : public QObject {
   Q_OBJECT
   QMutex mutable m_mutex;
   QByteArray m_buf;
   QBasicTimer m_copy, m_progress;
   QString m_error;
   QFile m_fi, m_fo;
   qint64 m_total, m_done;
   int m_shift;

   void close() {
      m_copy.stop();
      m_progress.stop();
      m_fi.close();
      m_fo.close();
      m_mutex.unlock();
   }
   /// Takes the error string from given file and emits an error indication.
   /// Closes the files and stops the copy. Always returns false
   bool error(QFile & f) {
      m_error = f.errorString();
      m_error.append(QStringLiteral("(in %1 file").arg(f.objectName()));
      emit finished(false, m_error);
      close();
      return false;
   }
   void finished() {
      emitProgress();
      emit finished(m_done == m_total, m_error);
      close();
   }
   void emitProgress() {
      emit progressed(m_done, m_total);
      emit hasProgressValue(m_done >> m_shift);
   }
   void timerEvent(QTimerEvent * ev) {
      if (ev->timerId() == m_copy.timerId()) {
         // Do the copy
         qint64 read = m_fi.read(m_buf.data(), m_buf.size());
         if (read == -1) { error(m_fi); return; }
         if (read == 0) return finished();
         qint64 written = m_fo.write(m_buf.constData(), read);
         if (written == -1) { error(m_fo); return; }
         Q_ASSERT(written == read);
         m_done += read;
      }
      else if (ev->timerId() == m_progress.timerId())
         emitProgress();
   }
   Q_INVOKABLE void cancelImpl() {
      if (!m_fi.isOpen()) return;
      m_error = "Canceled";
      finished();
   }
public:
   explicit FileCopier(QObject * parent = 0) :
      QObject(parent),
      // Copy 64kbytes at a time. On a modern hard drive, we'll copy
      // on the order of 1000 such blocks per second.
      m_buf(65536, Qt::Uninitialized)
   {
      m_fi.setObjectName("source");
      m_fo.setObjectName("destination");
   }
   /// Copies a file to another with progress indication.
   /// Returns false if the files cannot be opened.
   /// This method is thread safe.
   Q_SLOT bool copy(const QString & src, const QString & dst) {
      bool locked = m_mutex.tryLock();
      Q_ASSERT_X(locked, "copy",
                 "Another copy is already in progress");
      m_error.clear();

      // Open the files
      m_fi.setFileName(src);
      m_fo.setFileName(dst);
      if (! m_fi.open(QIODevice::ReadOnly)) return error(m_fi);
      if (! m_fo.open(QIODevice::WriteOnly)) return error(m_fo);
      m_total = m_fi.size();
      if (m_total < 0) return error(m_fi);

      // File size might not fit into an integer, calculate the number of
      // binary digits to shift it right by. Recall that QProgressBar etc.
      // all use int, not qint64!
      m_shift = 0;
      while ((m_total>>m_shift) >= std::numeric_limits<int>::max()) m_shift++;
      emit hasProgressMaximum(m_total>>m_shift);

      m_done = 0;
      m_copy.start(0, this);
      m_progress.start(100, this); // Progress is emitted at 10Hz rate
      return true;
   }
   /// This method is thread safe only when a copy is not in progress.
   QString lastError() const {
      bool locked = m_mutex.tryLock();
      Q_ASSERT_X(locked, "lastError",
                 "A copy is in progress. This method can only be used when"
                 "a copy is done");
      QString error = m_error;
      m_mutex.unlock();
      return error;
   }
   /// Cancels a pending copy operation. No-op if no copy is underway.
   /// This method is thread safe.
   Q_SLOT void cancel() {
      QMetaObject::invokeMethod(this, "cancelImpl");
   }
   /// Signal for progress indication with number of bytes
   Q_SIGNAL void progressed(qint64 done, qint64 total);
   /// Signals for progress that uses abstract integer values
   Q_SIGNAL void hasProgressMaximum(int total);
   Q_SIGNAL void hasProgressValue(int done);
   ///
   Q_SIGNAL void finished(bool ok, const QString & error);
};

/// A thread that is always destructible: if quits the event loop and waits
/// for it to finish.
class Thread : public QThread {
public:
   ~Thread() { quit(); wait(); }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QString src = QFileDialog::getOpenFileName(0, "Source File");
   if (src.isEmpty()) return 1;
   QString dst = QFileDialog::getSaveFileName(0, "Destination File");
   if (dst.isEmpty()) return 1;

   QProgressDialog dlg("File Copy Progress", "Cancel", 0, 100);
   Q_ASSERT(!dlg.isModal());
   Thread thread;
   FileCopier copier;
   copier.moveToThread(&thread);
   thread.start();
   dlg.connect(&copier, SIGNAL(hasProgressMaximum(int)),
               SLOT(setMaximum(int)));
   dlg.connect(&copier, SIGNAL(hasProgressValue(int)),
               SLOT(setValue(int)));
   copier.connect(&dlg, SIGNAL(canceled()), SLOT(cancel()));
   a.connect(&copier, SIGNAL(finished(bool,QString)), SLOT(quit()));
   // The copy method is thread safe.
   copier.copy(src, dst);
   return a.exec();
}

#include "main.moc"
