// https://github.com/KubaO/stackoverflown/tree/master/questions/loggers-22862059
#include <QtWidgets>
#include <cstdio>

/// An interface to be implemented by loggers.
class Logger : public QObject
{
   Q_OBJECT
public:
   Logger(QObject *parent = {});
   Q_SLOT virtual void Log(const QString &txt) = 0;
};

Logger::Logger(QObject *parent) : QObject(parent) {}

/// Logs to a QPlainTextEdit
class TextBoxGUILogger : public Logger
{
   Q_OBJECT
public:
   TextBoxGUILogger(QObject *parent = {});
   void setWidget(QPlainTextEdit *);
   void Log(const QString &txt) override;
private:
   QPointer<QPlainTextEdit> m_txtEditBox;
};

TextBoxGUILogger::TextBoxGUILogger(QObject *parent) : Logger(parent) {}

void TextBoxGUILogger::setWidget(QPlainTextEdit *edit) {
   m_txtEditBox = edit;
}

void TextBoxGUILogger::Log(const QString &txt) {
   if (m_txtEditBox) m_txtEditBox->appendPlainText(txt);
}

/// Logs to the standard output.
class CLILogger : public Logger
{
   Q_OBJECT
public:
   CLILogger(QObject *parent = {});
   void Log(const QString &txt) override;
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
   FileLogger(QObject *parent = {});
   /// The file can be owned by another object, or it can be made a child
   /// of the logger. In either case the behavior will be correct.
   void setFile(QFile *);
   void Log(const QString &txt) override;
private:
   QPointer<QFile> m_file;
};

FileLogger::FileLogger(QObject *parent) : Logger(parent) {}

void FileLogger::setFile(QFile *file) {
   m_file = file;
}

void FileLogger::Log(const QString &txt) {
   if (m_file) m_file->write(txt.toLocal8Bit().constData());
}


int main() {}

#include "main.moc"
