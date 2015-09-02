#include <QApplication>
#include <QToolButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QStyle>
#include <QStyleOptionButton>
#include <QMenu>
#include <QPushButton>
#include <QKeyEvent>
#include <QDebug>

class EditButton : public QToolButton {
    Q_OBJECT
    QLineEdit * m_edit;
    QRect m_textGeometry;
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY newText USER true)
public:
    EditButton(QWidget * parent = 0) : QToolButton(parent),
        m_edit(new QLineEdit(this))
    {
        m_edit->hide();
        connect(m_edit, SIGNAL(textChanged(QString)), SIGNAL(newText(QString)));
        connect(m_edit, SIGNAL(editingFinished()), SLOT(hideEditor()));
        setFocusPolicy(Qt::StrongFocus);
    }
    QString text() const { return m_edit->text(); }
    void setText(const QString & text) {
        if (text == m_edit->text()) return;
        m_edit->setText(text);
        QToolButton::setText(text);
        emit newText(text);
    }
    Q_SIGNAL void newText(const QString &);
protected:
    void resizeEvent(QResizeEvent * ev) {
        QToolButton::resizeEvent(ev);
        setEditGeometry();
    }
    void keyPressEvent(QKeyEvent * ev) {
        if (ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return) {
            showEditor();
            return;
        }
        QToolButton::keyPressEvent(ev);
    }
    void enterEvent(QEvent * ev) {
        showEditor();
        QToolButton::enterEvent(ev);
    }
    void leaveEvent(QEvent * ev) {
        hideEditor();
        QToolButton::leaveEvent(ev);
    }
private:
    Q_SLOT void hideEditor() {
        QToolButton::setText(m_edit->text());
        setFocusProxy(0);
        m_edit->hide();
        update();
    }
    void showEditor() {
        setEditGeometry();
        m_edit->show();
        setFocusProxy(m_edit);
        setFocus();
    }
    void setEditGeometry() {
        QStyleOptionToolButton opt;
        initStyleOption(&opt);
        QRect r = style()->subControlRect(QStyle::CC_ToolButton, &opt, QStyle::SC_ToolButton, this);
        m_edit->setGeometry(r);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWidget window;
    QGridLayout * l = new QGridLayout(&window);
    EditButton * btn = new EditButton;
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QMenu * menu = new QMenu(btn);
    menu->addAction("Action!");
    btn->setMenu(menu);
    btn->setPopupMode(QToolButton::MenuButtonPopup);
    btn->setText("Foo Bar Baz");
    l->addWidget(btn);
    l->addWidget(new QPushButton("Focus Test Button"));
    window.show();
    return a.exec();
}

#include "main.moc"
