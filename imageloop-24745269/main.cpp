#include <QApplication>
#include <QImage>
#include <QGridLayout>
#include <QLabel>
#include <QBasicTimer>

class Widget : public QWidget {
    QGridLayout m_layout;
    QLabel m_name, m_image;
    QStringList m_images;
    QStringList::const_iterator m_imageIt;
    QBasicTimer m_timer;
    void timerEvent(QTimerEvent * ev) {
        if (ev->timerId() == m_timer.timerId()) tick();
    }
    void tick() {
        display(*m_imageIt);
        m_imageIt ++;
        const bool loop = false;
        if (m_imageIt == m_images.end()) {
            if (loop)
                m_imageIt = m_images.begin();
            else
                m_timer.stop();
        }
    }
    void display(const QString & imageName) {
        QImage img(":/images/" + imageName);
        m_name.setText(imageName);
        m_image.setPixmap(QPixmap::fromImage(img));
    }
public:
    Widget(QWidget * parent = 0) : QWidget(parent), m_layout(this) {
        m_images << "redScreen.png" << "blueScreen.png" << "greenScreen.png";
        m_imageIt = m_images.begin();
        m_layout.addWidget(&m_name, 0, 0);
        m_layout.addWidget(&m_image, 1, 0);
        tick();
        m_timer.start(5000, Qt::CoarseTimer, this);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
