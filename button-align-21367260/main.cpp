#include <QPushButton>
#include <QGridLayout>
#include <QApplication>

class AlignButton : public QPushButton {
    Q_OBJECT
    Qt::Alignment m_alignment;
    Q_SLOT void clicked()  {
        m_alignment ^= Qt::AlignCenter;
        parentWidget()->layout()->setAlignment(this, m_alignment);
        label();
    }
    void label() {
        setText(QString("Alignment = %1").arg(m_alignment));
    }
public:
    AlignButton(Qt::Alignment alignment, QWidget * parent = 0) :
        QPushButton(parent),
        m_alignment(alignment)
    {
        connect(this, SIGNAL(clicked()), SLOT(clicked()));
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        label();
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QWidget window;
    QGridLayout * layout = new QGridLayout(&window);
    layout->addWidget(new AlignButton(0), 0, 0, 0);
    layout->addWidget(new AlignButton(Qt::AlignCenter), 1, 0, Qt::AlignCenter);
    window.setMinimumSize(500, 200);
    window.show();
    return app.exec();
}

#include "main.moc"
