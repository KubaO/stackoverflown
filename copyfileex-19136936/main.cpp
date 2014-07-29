#include <QApplication>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QThread>
#include <QPointer>
#include <QStateMachine>
#include <windows.h>

static QString getLastErrorMsg() {
    LPWSTR bufPtr = NULL;
    DWORD err = GetLastError();
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                   FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err, 0, (LPWSTR)&bufPtr, 0, NULL);
    const QString result = 
        (bufPtr) ? QString::fromUtf16((const ushort*)bufPtr).trimmed() :
                   QString("Unknown Error %1").arg(err);
    LocalFree(bufPtr);
    return result;
}
class CopierWorker : public QThread { // only to be used by the Copier object
   BOOL m_stop;
   QString m_src, m_dst;
   QPointer<QObject> m_object;
   static DWORD CALLBACK copyProgress(
         LARGE_INTEGER totalSize, LARGE_INTEGER totalTransferred,
         LARGE_INTEGER streamSize, LARGE_INTEGER streamTransferred,
         DWORD streamNo, DWORD callbackReason, HANDLE src, HANDLE dst,
         LPVOID data)
   {
      Q_UNUSED(streamSize) Q_UNUSED(streamTransferred)
      Q_UNUSED(streamNo) Q_UNUSED(callbackReason)
      Q_UNUSED(src) Q_UNUSED(dst)
      QObject * object = static_cast<QObject*>(data);
      const QString text = QString("Transferred %1 of %2 bytes").
            arg(totalTransferred.QuadPart).arg(totalSize.QuadPart);
      QMetaObject::invokeMethod(object, "newStatus", Qt::QueuedConnection,
                                Q_ARG(QString, text));
      return PROGRESS_CONTINUE;
   }
   void run() {
      m_stop = FALSE;
      BOOL rc = CopyFileExW((LPCWSTR)m_src.utf16(), (LPCWSTR)m_dst.utf16(),
                            &copyProgress, m_object, &m_stop, 0);
      if (!rc) {
         QMetaObject::invokeMethod(m_object, "newStatus", Qt::QueuedConnection,
                                   Q_ARG(QString, getLastErrorMsg()));
      }
   }
   CopierWorker(const QString & src, const QString & dst, QObject * obj) :
      m_src(src), m_dst(dst), m_object(obj) {}
   void stop() { m_stop = TRUE; }
   friend class Copier;
};

class Copier : public QObject {
   Q_OBJECT
   QPointer<CopierWorker> m_worker;
public:
   Copier(const QString & src, const QString & dst) : m_worker(new CopierWorker(src, dst, this)) {
      connect(m_worker, SIGNAL(finished()), SIGNAL(finished()));
      connect(m_worker, SIGNAL(finished()), m_worker, SLOT(deleteLater()));
   }
   ~Copier() {
      if (!m_worker) return;
      m_worker->stop();
      if (!m_worker->isRunning()) delete m_worker;
   }
   Q_SIGNAL void newStatus(const QString &);
   Q_SIGNAL void finished();
   Q_SLOT void stop() { m_worker->stop(); }
   void copy() { m_worker->start(); }
};

class Widget : public QWidget {
   Q_OBJECT
   QLabel * m_status;
   QLineEdit * m_src, * m_dst;

   Q_SIGNAL void stopCopy();
   Q_SLOT void startCopy() {
      Copier * copier = new Copier(m_src->text(), m_dst->text());
      connect(copier, SIGNAL(newStatus(QString)), m_status, SLOT(setText(QString)));
      connect(copier, SIGNAL(finished()), SIGNAL(copyFinished()));
      connect(this, SIGNAL(stopCopy()), copier, SLOT(stop()));
      copier->copy();
   }
   Q_SIGNAL void copyFinished();
public:
   Widget(QWidget * parent = 0) : QWidget(parent) {
      QGridLayout * layout = new QGridLayout(this);
      m_src = new QLineEdit;
      m_dst = new QLineEdit;
      QPushButton * srcButton = new QPushButton("...");
      QPushButton * dstButton = new QPushButton("...");
      QPushButton * copy = new QPushButton("Copy");
      QPushButton * cancel = new QPushButton("Cancel");
      QFileDialog * getSrc = new QFileDialog(this, "Source File");
      QFileDialog * getDst = new QFileDialog(this, "Source File");
      m_status = new QLabel;
      layout->addWidget(new QLabel("From:"), 0, 0, 1, 1, Qt::AlignRight);
      layout->addWidget(m_src, 0, 1, 1, 1);
      layout->addWidget(srcButton, 0, 2, 1, 1);
      layout->addWidget(new QLabel("To:"), 1, 0, 1, 1, Qt::AlignRight);
      layout->addWidget(m_dst, 1, 1, 1, 1);
      layout->addWidget(dstButton, 1, 2, 1, 1);
      layout->addWidget(m_status, 2, 0, 1, 3);
      layout->addWidget(copy, 3, 0, 1, 3, Qt::AlignCenter);
      layout->addWidget(cancel, 4, 0, 1, 3, Qt::AlignCenter);

      getSrc->setFileMode(QFileDialog::ExistingFile);
      getDst->setAcceptMode(QFileDialog::AcceptSave);
      connect(srcButton, SIGNAL(clicked()), getSrc, SLOT(show()));
      connect(dstButton, SIGNAL(clicked()), getDst, SLOT(show()));
      connect(getSrc, SIGNAL(fileSelected(QString)), m_src, SLOT(setText(QString)));
      connect(getDst, SIGNAL(fileSelected(QString)), m_dst, SLOT(setText(QString)));

      QStateMachine * machine = new QStateMachine(this);
      QState * stopped = new QState(machine);
      QState * copying = new QState(machine);
      machine->setInitialState(stopped);
      stopped->addTransition(copy, SIGNAL(clicked()), copying);
      stopped->assignProperty(copy, "enabled", true);
      stopped->assignProperty(cancel, "enabled", false);
      copying->addTransition(cancel, SIGNAL(clicked()), stopped);
      copying->addTransition(this, SIGNAL(copyFinished()), stopped);
      connect(copying, SIGNAL(entered()), SLOT(startCopy()));
      connect(copying, SIGNAL(exited()), SIGNAL(stopCopy()));
      copying->assignProperty(copy, "enabled", false);
      copying->assignProperty(cancel, "enabled", true);
      machine->start();
   }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   Widget w;
   w.show();
   return a.exec();
}

#include "main.moc"
