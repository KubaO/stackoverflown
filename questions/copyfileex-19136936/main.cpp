// https://github.com/KubaO/stackoverflown/tree/master/questions/copyfileex-19136936
#include <QtGui>
#include <QtConcurrent>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif
#include <windows.h>
#include <comdef.h>
//#define _WIN32_WINNT _WIN32_WINNT_WIN7

static QString toString(HRESULT hr) {
   _com_error err{hr};
   return QStringLiteral("Error 0x%1: %2").arg((quint32)hr, 8, 16, QLatin1Char('0'))
         .arg(err.ErrorMessage());
}

static QString getLastErrorMsg() {
   return toString(HRESULT_FROM_WIN32(GetLastError()));
}

static QString progressMessage(ULONGLONG part, ULONGLONG whole) {
   return QStringLiteral("Transferred %1 of %2 bytes.")
         .arg(part).arg(whole);
}

class Copier : public QObject {
   Q_OBJECT

   BOOL m_stop;
   QMutex m_pauseMutex;
   QAtomicInt m_pause;
   QWaitCondition m_pauseWait;

   QString m_src, m_dst;
   ULONGLONG m_lastPart, m_lastWhole;
   void newStatus(ULONGLONG part, ULONGLONG whole) {
      if (part != m_lastPart || whole != m_lastWhole) {
         m_lastPart = part;
         m_lastWhole = whole;
         emit newStatus(progressMessage(part, whole));
      }
   }
#if _WIN32_WINNT >= _WIN32_WINNT_WIN8
   static COPYFILE2_MESSAGE_ACTION CALLBACK copyProgress2(
         const COPYFILE2_MESSAGE *message, PVOID context);
#else
   static DWORD CALLBACK copyProgress(
         LARGE_INTEGER totalSize, LARGE_INTEGER totalTransferred,
         LARGE_INTEGER streamSize, LARGE_INTEGER streamTransferred,
         DWORD streamNo, DWORD callbackReason, HANDLE src, HANDLE dst,
         LPVOID data);
#endif
public:
   Copier(const QString & src, const QString & dst, QObject * parent = nullptr) :
      QObject{parent}, m_src{src}, m_dst{dst} {}
   Q_SIGNAL void newStatus(const QString &);
   Q_SIGNAL void finished();
   /// This method is thread-safe
   Q_SLOT void copy();
   /// This method is thread-safe
   Q_SLOT void stop() {
      resume();
      m_stop = TRUE;
   }
   /// This method is thread-safe
   Q_SLOT void pause() {
      m_pause = true;
   }
   /// This method is thread-safe
   Q_SLOT void resume() {
      if (m_pause)
         m_pauseWait.notify_one();
      m_pause = false;
   }
   ~Copier() override { stop(); }
};

#if _WIN32_WINNT >= _WIN32_WINNT_WIN8
void Copier::copy() {
   m_lastPart = m_lastWhole = {};
   m_stop = FALSE;
   m_pause = false;
   QtConcurrent::run([this]{
      COPYFILE2_EXTENDED_PARAMETERS params{
         sizeof(COPYFILE2_EXTENDED_PARAMETERS), 0, &m_stop,
               Copier::copyProgress2, this
      };
      auto rc = CopyFile2((PCWSTR)m_src.utf16(), (PCWSTR)m_dst.utf16(), &params);
      if (!SUCCEEDED(rc))
         emit newStatus(toString(rc));
      emit finished();
   });
}
COPYFILE2_MESSAGE_ACTION CALLBACK Copier::copyProgress2(
      const COPYFILE2_MESSAGE *message, PVOID context)
{
   COPYFILE2_MESSAGE_ACTION action = COPYFILE2_PROGRESS_CONTINUE;
   auto self = static_cast<Copier*>(context);
   if (message->Type == COPYFILE2_CALLBACK_CHUNK_FINISHED) {
      auto &info = message->Info.ChunkFinished;
      self->newStatus(info.uliTotalBytesTransferred.QuadPart, info.uliTotalFileSize.QuadPart);
   }
   else if (message->Type == COPYFILE2_CALLBACK_ERROR) {
      auto &info = message->Info.Error;
      self->newStatus(info.uliTotalBytesTransferred.QuadPart, info.uliTotalFileSize.QuadPart);
      emit self->newStatus(toString(info.hrFailure));
      action = COPYFILE2_PROGRESS_CANCEL;
   }
   if (self->m_pause) {
      QMutexLocker lock{&self->m_pauseMutex};
      self->m_pauseWait.wait(&self->m_pauseMutex);
   }
   return action;
}
#else
void Copier::copy() {
   m_lastPart = m_lastWhole = {};
   m_stop = FALSE;
   m_pause = false;
   QtConcurrent::run([this]{
      auto rc = CopyFileExW((LPCWSTR)m_src.utf16(), (LPCWSTR)m_dst.utf16(),
                            &copyProgress, this, &m_stop, 0);
      if (!rc)
         emit newStatus(getLastErrorMsg());
      emit finished();
   });
}
DWORD CALLBACK Copier::copyProgress(
      const LARGE_INTEGER totalSize, const LARGE_INTEGER totalTransferred,
      LARGE_INTEGER, LARGE_INTEGER, DWORD,
      DWORD, HANDLE, HANDLE,
      LPVOID data)
{
   auto self = static_cast<Copier*>(data);
   self->newStatus(totalTransferred.QuadPart, totalSize.QuadPart);
   if (self->m_pause) {
      QMutexLocker lock{&self->m_pauseMutex};
      self->m_pauseWait.wait(&self->m_pauseMutex);
   }
   return PROGRESS_CONTINUE;
}
#endif

struct PathWidget : public QWidget {
   QHBoxLayout layout{this};
   QLineEdit edit;
   QPushButton select{"..."};
   QFileDialog dialog;
   explicit PathWidget(const QString & caption) : dialog{this, caption} {
      layout.setMargin(0);
      layout.addWidget(&edit);
      layout.addWidget(&select);
      connect(&select, SIGNAL(clicked()), &dialog, SLOT(show()));
      connect(&dialog, SIGNAL(fileSelected(QString)), &edit, SLOT(setText(QString)));
   }
};

class Ui : public QWidget {
   Q_OBJECT
   QFormLayout m_layout{this};
   QPlainTextEdit m_status;
   PathWidget m_src{"Source File"}, m_dst{"Destination File"};
   QPushButton m_copy{"Copy"};
   QPushButton m_cancel{"Cancel"};

   QStateMachine m_machine{this};
   QState s_stopped{&m_machine};
   QState s_copying{&m_machine};

   Q_SIGNAL void stopCopy();
   Q_SLOT void startCopy() {
      auto copier = new Copier(m_src.edit.text(), m_dst.edit.text(), this);
      connect(copier, SIGNAL(newStatus(QString)), &m_status, SLOT(appendPlainText(QString)));
      connect(copier, SIGNAL(finished()), SIGNAL(copyFinished()));
      connect(copier, SIGNAL(finished()), copier, SLOT(deleteLater()));
      connect(this, SIGNAL(stopCopy()), copier, SLOT(stop()));
      copier->copy();
   }
   Q_SIGNAL void copyFinished();
public:
   Ui() {
      m_layout.addRow("From:", &m_src);
      m_layout.addRow("To:", &m_dst);
      m_layout.addRow(&m_status);
      m_layout.addRow(&m_copy);
      m_layout.addRow(&m_cancel);

      m_src.dialog.setFileMode(QFileDialog::ExistingFile);
      m_dst.dialog.setAcceptMode(QFileDialog::AcceptSave);
      m_status.setReadOnly(true);
      m_status.setMaximumBlockCount(5);

      m_machine.setInitialState(&s_stopped);
      s_stopped.addTransition(&m_copy, SIGNAL(clicked()), &s_copying);
      s_stopped.assignProperty(&m_copy, "enabled", true);
      s_stopped.assignProperty(&m_cancel, "enabled", false);
      s_copying.addTransition(&m_cancel, SIGNAL(clicked()), &s_stopped);
      s_copying.addTransition(this, SIGNAL(copyFinished()), &s_stopped);
      connect(&s_copying, SIGNAL(entered()), SLOT(startCopy()));
      connect(&s_copying, SIGNAL(exited()), SIGNAL(stopCopy()));
      s_copying.assignProperty(&m_copy, "enabled", false);
      s_copying.assignProperty(&m_cancel, "enabled", true);
      m_machine.start();
   }
};

int main(int argc, char *argv[])
{
   QApplication a{argc, argv};
   Ui ui;
   ui.show();
   return a.exec();
}
#include "main.moc"
