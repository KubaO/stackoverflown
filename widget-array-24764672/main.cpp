#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <array>

class WidgetCpp11 : public QWidget {
    QVBoxLayout m_layout;
    std::array<QLabel, 3> m_labels; // No overhead compared to `QLabel m_labels[3]`
    void setupLabels() {
        int n = 1;
        for (QLabel & label: m_labels) {
            label.setText(QString("Label %1").arg(n++));
            m_layout.addWidget(&label);
        }
    }
public:
    WidgetCpp11(QWidget * parent = 0) : QWidget(parent), m_layout(this) {
        setupLabels();
    }
};

class WidgetCpp98 : public QWidget {
    QVBoxLayout m_layout;
    QLabel m_labels[3];
    void setupLabels() {
        for (uint i = 0; i < sizeof(m_labels)/sizeof(m_labels[0]); ++i) {
            m_labels[i].setText(QString("Label %1").arg(i+1));
            m_layout.addWidget(m_labels+i);
        }
    }
public:
    WidgetCpp98(QWidget * parent = 0) : QWidget(parent), m_layout(this) {
        setupLabels();
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WidgetCpp98 w;
    w.show();
    return a.exec();
}
