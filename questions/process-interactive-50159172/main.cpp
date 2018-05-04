// https://github.com/KubaO/stackoverflown/tree/master/questions/process-interactive-50159172
#include <QtWidgets>
#include <algorithm>
#include <initializer_list>

class Commander : public QObject {
   Q_OBJECT
   QProcess m_process{this};
   QByteArrayList m_commands;
   QByteArrayList::const_iterator m_cmd = m_commands.cbegin();
   QByteArray m_log;
   QByteArray m_prompt;
   void onStdOut() {
      auto const chunk = m_process.readAllStandardOutput();
      m_log.append(chunk);
      emit hasStdOut(chunk);
      if (m_log.endsWith(m_prompt) && m_cmd != m_commands.end()) {
         m_process.write(*m_cmd);
         m_log.append(*m_cmd);
         emit hasStdIn(*m_cmd);
         if (m_cmd++ == m_commands.end())
            emit commandsDone();
      }
   }
public:
   Commander(QString program, QStringList arguments, QObject * parent = {}) :
      QObject(parent) {
      connect(&m_process, &QProcess::stateChanged, this, &Commander::stateChanged);
      connect(&m_process, &QProcess::readyReadStandardError, this, [this]{
         auto const chunk = m_process.readAllStandardError();
         m_log.append(chunk);
         emit hasStdErr(chunk);
      });
      connect(&m_process, &QProcess::readyReadStandardOutput, this, &Commander::onStdOut);
      connect(&m_process, &QProcess::errorOccurred, this, &Commander::hasError);
      m_process.setProgram(std::move(program));
      m_process.setArguments(std::move(arguments));
   }
   void setPrompt(QByteArray prompt) { m_prompt = std::move(prompt); }
   void setCommands(std::initializer_list<const char*> commands) {
      QByteArrayList l;
      l.reserve(int(commands.size()));
      for (auto c : commands) l << c;
      setCommands(l);
   }
   void setCommands(QByteArrayList commands) {
      Q_ASSERT(isIdle());
      m_commands = std::move(commands);
      m_cmd = m_commands.begin();
      for (auto &cmd : m_commands)
         cmd.append('\n');
   }
   void start() {
      Q_ASSERT(isIdle());
      m_cmd = m_commands.begin();
      m_process.start(QIODevice::ReadWrite | QIODevice::Text);
   }
   QByteArray log() const { return m_log; }
   QProcess::ProcessError error() const { return m_process.error(); }
   QProcess::ProcessState state() const { return m_process.state(); }
   int exitCode() const { return m_process.exitCode(); }
   Q_SIGNAL void stateChanged(QProcess::ProcessState);
   bool isIdle() const { return state() == QProcess::NotRunning; }
   Q_SIGNAL void hasError(QProcess::ProcessError);
   Q_SIGNAL void hasStdIn(const QByteArray &);
   Q_SIGNAL void hasStdOut(const QByteArray &);
   Q_SIGNAL void hasStdErr(const QByteArray &);
   Q_SIGNAL void commandsDone();
   ~Commander() {
      m_process.close(); // kill the process
   }
};

template <typename T> void forEachLine(const QByteArray &chunk, T &&fun) {
   auto start = chunk.begin();
   while (start != chunk.end()) {
      auto end = std::find(start, chunk.end(), '\n');
      auto lineEnds = end != chunk.end();
      fun(lineEnds, QByteArray::fromRawData(&*start, end-start));
      start = end;
      if (lineEnds) start++;
   }
}

class Logger : public QObject {
   Q_OBJECT
   QtMessageHandler previous = {};
   QTextCharFormat logFormat;
   bool lineStart = true;
   static QPointer<Logger> &instance() { static QPointer<Logger> ptr; return ptr; }
public:
   explicit Logger(QObject *parent = {}) : QObject(parent) {
      Q_ASSERT(!instance());
      instance() = this;
      previous = qInstallMessageHandler(Logger::logMsg);
   }
   void operator()(const QByteArray &chunk, const QTextCharFormat &modifier = {}) {
      forEachLine(chunk, [this, &modifier](bool ends, const QByteArray &chunk){
         auto text = QString::fromLocal8Bit(chunk);
         addText(text, modifier, lineStart);
         lineStart = ends;
      });
   }
   static void logMsg(QtMsgType, const QMessageLogContext &, const QString &msg) {
      (*instance())(msg.toLocal8Bit().append('\n'), instance()->logFormat);
   }
   Q_SIGNAL void addText(const QString &text, const QTextCharFormat &modifier, bool newBlock);
   void setLogFormat(const QTextCharFormat &format) { logFormat = format; }
   ~Logger() override { if (previous) qInstallMessageHandler(previous); }
};

static struct SystemFixedPitchFont_t {} constexpr SystemFixedPitchFont;
QTextCharFormat operator<<(QTextCharFormat format, const QBrush &brush) {
   return format.setForeground(brush), format;
}
QTextCharFormat operator<<(QTextCharFormat format, SystemFixedPitchFont_t) {
   return format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont)), format;
}

void addText(QPlainTextEdit *view, const QString &text, const QTextCharFormat &modifier, bool newBlock) {
   view->mergeCurrentCharFormat(modifier);
   if (newBlock)
      view->appendPlainText(text);
   else
      view->textCursor().insertText(text);
}

int main(int argc, char *argv[]) {
   QApplication app{argc, argv};

   Commander cmdr{"python", {"test.py"}};
   cmdr.setPrompt("$ ");
   cmdr.setCommands({"help", "exec !2", "exec !0", "help", "exec !1", "exec !3", "quit"});

   QWidget w;
   QVBoxLayout layout{&w};
   QPlainTextEdit logView;
   QPushButton start{"Start"};
   Logger log{logView.document()};
   layout.addWidget(&logView);
   layout.addWidget(&start);
   logView.setMaximumBlockCount(1000);
   logView.setReadOnly(true);
   logView.setCurrentCharFormat(QTextCharFormat() << SystemFixedPitchFont);
   log.setLogFormat(QTextCharFormat() << Qt::darkGreen);

   QObject::connect(&log, &Logger::addText, &logView, [&logView](auto &text, auto &mod, auto block){
      addText(&logView, text, mod, block);
   });
   QObject::connect(&cmdr, &Commander::hasStdOut, &log, [&log](auto &chunk){ log(chunk, QTextCharFormat() << Qt::black); });
   QObject::connect(&cmdr, &Commander::hasStdErr, &log, [&log](auto &chunk){ log(chunk, QTextCharFormat() << Qt::red); });
   QObject::connect(&cmdr, &Commander::hasStdIn, &log, [&log](auto &chunk){ log(chunk, QTextCharFormat() << Qt::blue); });
   QObject::connect(&cmdr, &Commander::stateChanged, &start, [&start](auto state){
      qDebug() << state;
      start.setEnabled(state == QProcess::NotRunning);
   });
   QObject::connect(&start, &QPushButton::clicked, &cmdr, &Commander::start);

   w.show();
   return app.exec();
}

#include "main.moc"
