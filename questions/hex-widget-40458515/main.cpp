// https://github.com/KubaO/stackoverflown/tree/master/questions/hex-widget-40458515
#include <QtWidgets>
#include <algorithm>
#include <array>
#include <cmath>

static auto const CP437 = QStringLiteral(
            " ☺☻♥♦♣♠•◘○◙♂♀♪♫☼▶◀↕‼¶§▬↨↑↓→←∟↔▲▼"
            "␣!\"#$%&'()*+,-./0123456789:;<=>?"
            "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
            "`abcdefghijklmnopqrstuvwxyz{|}~ "
            "ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜ¢£¥₧ƒ"
            "áíóúñÑªº¿⌐¬½¼¡«»░▒▓│┤╡╢╖╕╣║╗╝╜╛┐"
            "└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀"
            "αßΓπΣσµτΦΘΩδ∞φε∩≡±≥≤⌠⌡÷≈°∙·√ⁿ²■ ");

class HexView : public QAbstractScrollArea {
    Q_OBJECT
    const int m_addressChars = 8;
    const int m_dataMargin = 4;
    const char * m_data = {};
    size_t m_dataSize = 0;
    size_t m_dataStart = 0;
    QSize m_glyphSize;
    QPointF m_glyphPos;
    int m_charsPerLine, m_lines;
    QMap<QChar, QImage> m_glyphs;
    QFont m_font{"Monaco"};
    QFontMetricsF m_fm{m_font};
    static inline QChar decode(char ch) { return CP437[uchar(ch)]; }
    inline int xStep() const { return m_glyphSize.width(); }
    inline int yStep() const { return m_glyphSize.height(); }
    const QImage &glyph(QChar ch) {
        auto &glyph = m_glyphs[ch];
        if (glyph.isNull()) {
            QPointF extent = m_fm.boundingRect(ch).translated(m_glyphPos).bottomRight();
            glyph = QImage(m_glyphSize, QImage::Format_ARGB32_Premultiplied);
            glyph.fill(Qt::transparent);
            QPainter p{&glyph};
            p.setPen(Qt::white);
            p.setFont(m_font);
            p.translate(m_glyphPos);
            p.scale(std::min(1.0, (m_glyphSize.width()-1)/extent.x()),
                    std::min(1.0, (m_glyphSize.height()-1)/extent.y()));
            p.drawText(QPointF{}, {ch});
        }
        return glyph;
    }
    void drawChar(const QPoint &pos, QChar ch, QColor fg, QColor bg, QPainter &p) {
        QRect rect(pos, m_glyphSize);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawImage(pos, glyph(ch));
        p.setCompositionMode(QPainter::CompositionMode_SourceOut);
        p.fillRect(rect, bg);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
        p.fillRect(rect, fg);
        p.setCompositionMode({});
    }
    void initData() {
        qreal width = viewport()->width() - m_addressChars*xStep() - m_dataMargin;
        m_charsPerLine = (width > 0.) ? width/xStep() : 0.;
        m_lines = viewport()->height()/yStep();
        if (m_charsPerLine && m_lines) {
            verticalScrollBar()->setRange(0, m_dataSize/m_charsPerLine);
            verticalScrollBar()->setValue(m_dataStart/m_charsPerLine);
        } else {
            verticalScrollBar()->setRange(0, 0);
        }
    }
protected:
    void paintEvent(QPaintEvent *ev) override {
        QElapsedTimer time;
        time.start();
        QPainter p{viewport()};
        QPoint pos;
        QPoint const step{xStep(), 0};
        auto dividerX = m_addressChars*xStep() + m_dataMargin/2.;
        p.drawLine(dividerX, 0, dividerX, viewport()->height());
        int offset = 0;
        QRect rRect = ev->rect();
        while (offset < m_charsPerLine*m_lines && m_dataStart + offset < m_dataSize) {
            const auto address = QString::number(m_dataStart + offset, 16);
            pos += step * (m_addressChars - address.size());
            for (auto c : address) {
                if (QRect(pos, m_glyphSize).intersects(rRect))
                    drawChar(pos, c, Qt::black, Qt::white, p);
                pos += step;
            }
            pos += {m_dataMargin, 0};
            auto bytes = std::min(m_dataSize - offset, (size_t)m_charsPerLine);
            for (int n = bytes; n; n--) {
                if (QRect(pos, m_glyphSize).intersects(rRect))
                    drawChar(pos, decode(m_data[m_dataStart + offset]), Qt::red, Qt::white, p);
                pos += step;
                offset ++;
            }
            pos = {0, pos.y() + yStep()};
        }
        newStatus(QStringLiteral("%1ms").arg(time.nsecsElapsed()/1e6));
    }
    void resizeEvent(QResizeEvent *) override {
        initData();
    }
    void scrollContentsBy(int, int dy) override {
        m_dataStart = verticalScrollBar()->value() * (size_t)m_charsPerLine;
        viewport()->scroll(0, dy * m_glyphSize.height(), viewport()->rect());
    }
public:
    HexView(QWidget * parent = nullptr) : HexView(nullptr, 0, parent) {}
    HexView(const char * data, size_t size, QWidget * parent = nullptr) :
        QAbstractScrollArea{parent}, m_data(data), m_dataSize(size)
    {
        QRectF glyphRectF{0., 0., 1., 1.};
        for (int i = 0x20; i < 0xE0; ++i)
            glyphRectF = glyphRectF.united(m_fm.boundingRect(CP437[i]));
        m_glyphPos = -glyphRectF.topLeft();
        m_glyphSize = QSize(std::ceil(glyphRectF.width()), std::ceil(glyphRectF.height()));
        initData();
    }
    void setData(const char * data, size_t size) {
        if (data == m_data && size == m_dataSize) return;
        m_data = data;
        m_dataSize = size;
        m_dataStart = 0;
        initData();
        viewport()->update();
    }
    Q_SIGNAL void newStatus(const QString &);
};

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    QFile file{app.applicationFilePath()};
    if (!file.open(QIODevice::ReadOnly)) return 1;
    auto *const map = (const char*)file.map(0, file.size(), QFile::MapPrivateOption);
    if (!map) return 2;

    QWidget ui;
    QGridLayout layout{&ui};
    HexView view;
    QRadioButton exe{"Executable"};
    QRadioButton charset{"Character Set"};
    QLabel status;
    layout.addWidget(&view, 0, 0, 1, 4);
    layout.addWidget(&exe, 1, 0);
    layout.addWidget(&charset, 1, 1);
    layout.addWidget(&status, 1, 2, 1, 2);
    QObject::connect(&exe, &QPushButton::clicked, [&]{
        view.setData(map, (size_t)file.size());
    });
    QObject::connect(&charset, &QPushButton::clicked, [&]{
        static std::array<char, 256> data;
        std::iota(data.begin(), data.end(), char(0));
        view.setData(data.data(), data.size());
    });
    QObject::connect(&view, &HexView::newStatus, &status, &QLabel::setText);
    charset.click();
    ui.resize(1000, 800);
    ui.show();
    return app.exec();
}

#include "main.moc"
