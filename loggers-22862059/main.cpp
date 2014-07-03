#include <QApplication>
#include <QTextEdit>
#include <QPointer>
#include <QFile>
#include <cstdio>

/// An interface to be implemented by loggers.
class Logger : public QObject
{
   Q_OBJECT

public:
   Logger(QObject * parent = 0);

public slots:
   virtual void Log(const QString& txt) = 0;
};

Logger::Logger(QObject *parent) : QObject(parent) {}

/// Logs to a QTextEdit
class TextBoxGUILogger : public Logger
{
   Q_OBJECT

public:
   TextBoxGUILogger(QObject * parent = 0);
   void setWidget(QTextEdit*);
   void Log(const QString& txt);

private:
   QPointer<QTextEdit> m_txtEditBox;
};

TextBoxGUILogger::TextBoxGUILogger(QObject * parent) : Logger(parent) {}

void TextBoxGUILogger::setWidget(QTextEdit * edit) {
   m_txtEditBox = edit;
}

void TextBoxGUILogger::Log(const QString &txt) {
   if (m_txtEditBox) m_txtEditBox->append(txt);
}

/// Logs to the standard output.
class CLILogger : public Logger
{
   Q_OBJECT

public:
   CLILogger(QObject * parent = 0);
   void Log(const QString& txt);
};

CLILogger::CLILogger(QObject *parent) : Logger(parent) {}

void CLILogger::Log(const QString &txt) {
   printf("%s", txt.toLocal8Bit().constData());
}

/// Logs to a file.
class FileLogger : public Logger
{
   Q_OBJECT

public:
   FileLogger(QObject * parent = 0);
   /// The file can be owned by another object, or it can be made a child
   /// of the logger. In either case the behavior will be correct.
   void setFile(QFile*);

public slots:
   void Log(const QString& txt);

private:
   QPointer<QFile> m_file;
};

FileLogger::FileLogger(QObject * parent) : Logger(parent) {}

void FileLogger::setFile(QFile * file) {
   m_file = file;
}

void FileLogger::Log(const QString &txt) {
   if (m_file) m_file->write(txt.toLocal8Bit().constData());
}


int main(int argc, char *argv[])
{
   QApplication a(argc, argv);

   return a.exec();
}

#include "main.moc"
