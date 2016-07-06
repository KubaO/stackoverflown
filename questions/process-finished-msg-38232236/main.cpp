// https://github.com/KubaO/stackoverflown/tree/master/questions/process-finished-msg-38232236
#include <QtWidgets>

class Window : public QWidget {
   QVBoxLayout m_layout{this};
   QPushButton m_button{tr("Sleep")};
   QMessageBox m_box{QMessageBox::Information,
            tr("Wakey-wakey"),
            tr("A process is done sleeping."),
            QMessageBox::Ok, this};
   QProcess m_process;
public:
   Window() {
      m_layout.addWidget(&m_button);
      m_process.setProgram("sleep");
      m_process.setArguments({"5"});
      connect(&m_button, &QPushButton::clicked, &m_process, [=]{ m_process.start(); });
      connect(&m_process, (void(QProcess::*)(int))&QProcess::finished, [=]{ m_box.show(); });
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Window window;
   window.show();
   return app.exec();
}
