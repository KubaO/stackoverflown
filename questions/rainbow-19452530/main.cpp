#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <QLabel>
#include <algorithm>
#include <cmath>

QColor wavelengthToColor(qreal lambda)
{
    // Based on: http://www.efg2.com/Lab/ScienceAndEngineering/Spectra.htm
    // The foregoing is based on: http://www.midnightkite.com/color.html
    struct Color {
    public:
        qreal c[3];
        QColor toColor(qreal factor) const {
            qreal const gamma = 0.8;
            int ci[3];
            for (int i = 0; i < 3; ++i) {
                ci[i] = c[i] == 0.0 ? 0.0 : qRound(255 * pow(c[i] * factor, gamma));
            }
            return QColor(ci[0], ci[1], ci[2]);
        }
    } color;
    qreal factor = 0.0;

    static qreal thresholds[] = { 380, 440, 490, 510, 580, 645, 780 };
    for (unsigned int i = 0; i < sizeof(thresholds)/sizeof(thresholds[0]); ++ i) {
        using std::swap;
        qreal t1 = thresholds[i], t2 = thresholds[i+1];
        if (lambda < t1 || lambda >= t2) continue;
        if (i%2) swap(t1, t2);
        color.c[i % 3] = (i < 5) ? (lambda - t2) / (t1-t2) : 0.0;;
        color.c[2-i/2] = 1.0;
        factor = 1.0;
        break;
    }

    // Let the intensity fall off near the vision limits
    if (lambda >= 380 && lambda < 420) {
        factor = 0.3 + 0.7*(lambda-380) / (420 - 380);
    }
    else if (lambda >= 700 && lambda < 780) {
        factor = 0.3 + 0.7*(780 - lambda) / (780 - 700);
    }
    return color.toColor(factor);
}

QPixmap rainbow(int w, int h)
{
    QPixmap pm(w, h);
    QPainter p(&pm);
    qreal f1 = 1.0/400;
    qreal f2 = 1.0/780;
    for (int x = 0; x < w; ++ x) {
        // Iterate across frequencies, not wavelengths
        qreal lambda = 1.0/(f1-(x/qreal(w)*(f1-f2)));
        p.setPen(wavelengthToColor(lambda));
        p.drawLine(x, 0, x, h);
    }
    return pm;
}

class RainbowLabel : public QLabel {
protected:
    void resizeEvent(QResizeEvent *) {
        setPixmap(rainbow(width(), height()));
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RainbowLabel l;
    l.resize(600, 100);
    l.show();
    return a.exec();
}
