#if 1

#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QWidget>
#include <cmath>

//! Fractional part of the argument, with the same sign as the argument.
template <typename T> inline T fract(const T & x) { return x-trunc(x); }
//! A rectangle moved to the fractional part of the original topLeft()
template <> inline QRectF fract(const QRectF & r) { return QRectF(fract(r.x()), fract(r.y()), r.width(), r.height()); }
//! An integral size that contains the size of a given rectangle.
static QSize ceil(const QRectF & r) { return QSize(ceil(r.width()), ceil(r.height())); }
//! An integral point obtained by rounding `p` towards zero.
static QPoint truncint(const QPointF & p) { return QPoint(trunc(p.x()), trunc(p.y())); }

class Widget : public QWidget {
    void paintEvent(QPaintEvent *) {
        static auto const text(QString(300, 'm'));
        QPainter p(this);
        p.setBrush(Qt::black);
        p.drawRect(rect());
        p.setPen(Qt::white);
        drawFadedLineText(&p, rect(), text);
    }
    // The semantics of `rect` and `s` are the same as in the call to `drawText(rect, s)`;
    void drawFadedLineText(QPainter* const painter, const QRectF & rect, const QString & s)
    {
        auto const fontMetrics(painter->fontMetrics());
        if (fontMetrics.width(s) <= rect.width()) {
            painter->drawText(rect, s);
            return;
        }

        static QLinearGradient lg;
        static bool init;
        if (!init) {
            init = true;
            lg.setStops(QGradientStops{
                            qMakePair(qreal(0), QColor(0, 0, 0, 238)),
                            qMakePair(qreal(1), QColor(0, 0, 0, 17))});
        }

        auto const margin(qreal(20.0));
        auto pixRect(fract(rect));
        auto const right(pixRect.right());
        lg.setStart(right - margin, 0.0);
        lg.setFinalStop(right, 0.0);

        QPixmap pixmap(ceil(rect));
        pixmap.fill(Qt::transparent);
        QPainter p(&pixmap);
        p.setPen(painter->pen());
        p.setFont(painter->font());
        p.drawText(pixRect, s);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(QRectF(right - margin, 0, margin, pixmap.rect().height()), lg);
        p.end();
        painter->drawPixmap(truncint(rect.topLeft()), pixmap);
    }
public:
    Widget() { setAttribute(Qt::WA_OpaquePaintEvent); setAttribute(Qt::WA_StaticContents); }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}

#else

#include <QApplication>
#include <QBackingStore>
#include <QFontMetrics>
#include <QPainter>
#include <QWidget>
#include <cmath>

//! Fractional part of the argument, with the same sign as the argument.
template <typename T> inline T fract(const T & x) { return x-trunc(x); }
//! A rectangle moved to the fractional part of the original topLeft()
template <> inline QRectF fract(const QRectF & r) { return QRectF(fract(r.x()), fract(r.y()), r.width(), r.height()); }
//! An integral size that contains the size of a given rectangle.
static QSize ceil(const QRectF & r) { return QSize(ceil(r.width()), ceil(r.height())); }
//! An integral point obtained by rounding `p` towards zero.
static QPoint truncint(const QPointF & p) { return QPoint(trunc(p.x()), trunc(p.y())); }

class Widget : public QWidget {
    void paintEvent(QPaintEvent *) {
        static auto const text(QString(300, 'm'));
        QPainter p(this);
        p.setBrush(Qt::black);
        p.drawRect(rect());
        p.setPen(Qt::white);
        drawFadedLineText(&p, rect(), text);
    }
    // The semantics of `rect` and `s` are the same as in the call to `drawText(rect, s)`;
    void drawFadedLineText(QPainter* const painter, const QRectF & rect, const QString & s)
    {
        auto const fontMetrics(painter->fontMetrics());
        if (fontMetrics.width(s) <= rect.width()) {
            painter->drawText(rect, s);
            return;
        }

        static QLinearGradient lg;
        static bool init;
        if (!init) {
            init = true;
            lg.setStops(QGradientStops{
                            qMakePair(qreal(0), QColor(0, 0, 0, 238)),
                            qMakePair(qreal(1), QColor(0, 0, 0, 17))});
        }

        auto const margin(qreal(20.0));
        auto pixRect(fract(rect));
        auto const right(pixRect.right());
        lg.setStart(right - margin, 0.0);
        lg.setFinalStop(right, 0.0);

        QPixmap pixmap(ceil(rect));
        pixmap.fill(Qt::transparent);
        QPainter p(&pixmap);
        p.setPen(painter->pen());
        p.setFont(painter->font());
        p.drawText(pixRect, s);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(QRectF(right - margin, 0, margin, pixmap.rect().height()), lg);
        p.end();
        painter->drawPixmap(truncint(rect.topLeft()), pixmap);
    }
public:
    Widget() { setAttribute(Qt::WA_OpaquePaintEvent); setAttribute(Qt::WA_StaticContents); }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}

#endif


