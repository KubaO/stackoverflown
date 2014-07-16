// forms.h - interface

#include <QScopedPointer>

class LoginForm;
class WorldForm;
class Forms {
  Q_DISABLE_COPY(Forms)
  QScopedPointer<LoginForm> m_loginForm;
  QScopedPointer<WorldForm> m_worldForm;
public:
  Forms();
  void login();
  void startGame();
  ~Forms();
};

// forms.cpp - implementation

#include <QWidget>
class LoginForm : public QWidget {};
class WorldForm : public QWidget {};

Forms::Forms() {}

void Forms::login() {
    if (!m_loginForm) m_loginForm.reset(new LoginForm);
    m_loginForm->show();
}

void Forms::startGame() {
    if (!m_worldForm) m_worldForm.reset(new WorldForm);
    m_worldForm->show();
    if (m_loginForm) m_loginForm->hide();
}

Forms::~Forms() {}

//

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    return a.exec();
}
