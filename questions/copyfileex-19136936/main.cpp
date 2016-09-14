// https://github.com/KubaO/stackoverflown/tree/master/questions/copyfileex-19136936
#include <QtGui>
#include <QtConcurrent>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif
#include <windows.h>

static QString getLastErrorMsg() {
    LPWSTR bufPtr = NULL;
    auto err = GetLastError();
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err, 0, (LPWSTR)&bufPtr, 0, NULL);
    const auto result =
        (bufPtr) ? QString::fromUtf16((const ushort*)bufPtr).trimmed() :
                   QString("Unknown Error %1").arg(err);
    LocalFree(bufPtr);
    return result;
}

class Copier : public QObject {
   Q_OBJECT
   BOOL m_stop;
   QString m_src, m_dst;
   static DWORD CALLBACK copyProgress(
         LARGE_INTEGER totalSize, LARGE_INTEGER totalTransferred,
         LARGE_INTEGER streamSize, LARGE_INTEGER streamTransferred,
         DWORD streamNo, DWORD callbackReason, HANDLE src, HANDLE dst,
         LPVOID data)
   {
      Q_UNUSED(streamSize) Q_UNUSED(streamTransferred)
      Q_UNUSED(streamNo) Q_UNUSED(callbackReason)
      Q_UNUSED(src) Q_UNUSED(dst)
      auto self = static_cast<Copier*>(data);
      const auto text = QString("Transferred %1 of %2 bytes").
            arg(totalTransferred.QuadPart).arg(totalSize.QuadPart);
      emit self->newStatus(text);
      return PROGRESS_CONTINUE;
   }
public:
   Copier(const QString & src, const QString & dst, QObject * parent = nullptr) :
      QObject{parent}, m_src{src}, m_dst{dst} {}
   Q_SIGNAL void newStatus(const QString &);
   Q_SIGNAL void finished();
   Q_SLOT void stop() { m_stop = TRUE; }
   void copy() {
      QtConcurrent::run([this]{
         m_stop = FALSE;
         auto rc = CopyFileExW((LPCWSTR)m_src.utf16(), (LPCWSTR)m_dst.utf16(),
                               &copyProgress, this, &m_stop, 0);
         if (!rc)
            emit newStatus(getLastErrorMsg());
         emit finished();
      });
   }
   ~Copier() { stop(); }
};

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
   QLabel m_status;
   PathWidget m_src{"Source File"}, m_dst{"Destination File"};
   QPushButton m_copy{"Copy"};
   QPushButton m_cancel{"Cancel"};

   QStateMachine m_machine{this};
   QState s_stopped{&m_machine};
   QState s_copying{&m_machine};

   Q_SIGNAL void stopCopy();
   Q_SLOT void startCopy() {
      auto copier = new Copier(m_src.edit.text(), m_dst.edit.text(), this);
      connect(copier, SIGNAL(newStatus(QString)), &m_status, SLOT(setText(QString)));
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
