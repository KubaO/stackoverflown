#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QObject>
#include <QSharedMemory>

class AuthenticationDialog : public QDialog {
    QString m_userName, m_password;
    QLineEdit m_userNameIn, m_passwordIn;
    QDialogButtonBox m_buttons;
public:
    AuthenticationDialog() : QDialog(),
    m_buttons(QDialogButtonBox::Ok | QDialogButtonBox::Close) {
        QFormLayout * layout = new QFormLayout(this);
        layout->addRow("Username", &m_userNameIn);
        layout->addRow("Password", &m_passwordIn);
        m_passwordIn.setEchoMode(QLineEdit::PasswordEchoOnEdit);
        //m_buttons.addButton();
        //connect(&m_buttons, SIGNAL(accepted()), SLOT
    }

};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    return a.exec();
}
