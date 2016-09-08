// https://github.com/KubaO/stackoverflown/tree/master/questions/simplepaint-39358392
#include <QtWidgets>

// Make a subclass of the QWidget class, so that you can override some of its
// virtual methods
class PaintWidget : public QWidget {
    // Create a QPixmap object that you will use to store the bitmap
    // that the user will draw [on].
    QPixmap m_pixmap;
    QPoint m_lastPos;
    // Override the paintEvent(QPaintEvent *) [...]
    void paintEvent(QPaintEvent *) override {
        QPainter painter{this};
        painter.drawPixmap(0, 0, m_pixmap);
    }
    void resizeEvent(QResizeEvent *) override {
        // [...] size the QPixmap to be at least as big as the maximum size of the window
        // We'll also never let it shrink so as not to lose the already drawn image.
        auto newRect = m_pixmap.rect().united(rect());
        if (newRect == m_pixmap.rect()) return;
        QPixmap newPixmap{newRect.size()};
        QPainter painter{&newPixmap};
        painter.fillRect(newPixmap.rect(), Qt::white);
        painter.drawPixmap(0, 0, m_pixmap);
        m_pixmap = newPixmap;
    }
    // Override the mousePressEvent(QMouseEvent *) [...]
    void mousePressEvent(QMouseEvent * ev) override {
        m_lastPos = ev->pos();
        draw(ev->pos());
    }
    // Override the mouseMoveEvent(QMouseEvent *) [...]
    void mouseMoveEvent(QMouseEvent * ev) override {
        draw(ev->pos());
    }
    void draw(const QPoint & pos) {
        QPainter painter{&m_pixmap};
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen({Qt::blue, 2.0});
        painter.drawLine(m_lastPos, pos);
        m_lastPos = pos;
        update();
    }
public:
    using QWidget::QWidget;
};

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    // Create an object of your subclass and call show()
    PaintWidget ui;
    ui.show();
    return app.exec();
}
