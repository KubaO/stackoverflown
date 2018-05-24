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
    const qreal m_dataMargin = 4.;
    const char * m_data = {};
    size_t m_dataSize = 0;
    size_t m_dataStart = 0;
    QRectF m_glyphRect{0.,0.,1.,1.};
    QPointF m_glyphPos;
    int m_chars, m_lines;
    QMap<QChar, QImage> m_glyphs;
    QFont m_font{"Monaco"};
    static inline QChar decode(char ch) { return CP437[uchar(ch)]; }
    inline qreal xStep() const { return m_glyphRect.width(); }
    inline qreal yStep() const { return m_glyphRect.height(); }
    const QImage &glyph(QChar ch) {
        auto &glyph = m_glyphs[ch];
        if (glyph.isNull()) {
            glyph = QImage{m_glyphRect.size().toSize(), QImage::Format_ARGB32_Premultiplied};
            glyph.fill(Qt::transparent);
            QPainter p{&glyph};
            p.setPen(Qt::white);
            p.setFont(m_font);
            p.drawText(m_glyphPos, {ch});
        }
        return glyph;
    }
    void drawChar(const QPointF &pos, QChar ch, QColor fg, QColor bg, QPainter &p) {
        auto rect = m_glyphRect;
        rect.moveTo(pos);
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
        m_chars = (width > 0.) ? width/xStep() : 0.;
        m_lines = viewport()->height()/yStep();
        if (m_chars && m_lines) {
            verticalScrollBar()->setRange(0, m_dataSize/m_chars);
            verticalScrollBar()->setValue(m_dataStart/m_chars);
        } else {
            verticalScrollBar()->setRange(0, 0);
        }
    }    void paintEvent(QPaintEvent *) override {
        QElapsedTimer time;
        time.start();
        QPainter p{viewport()};
        QPointF pos;
        QPointF const step{xStep(), 0.};
        auto dividerX = m_addressChars*xStep() + m_dataMargin/2.;
        p.drawLine(dividerX, 0, dividerX, viewport()->height());
        int offset = 0;
        while (offset < m_chars*m_lines && m_dataStart + offset < m_dataSize) {
            const auto address = QString::number(m_dataStart + offset, 16);
            pos += step * (m_addressChars - address.size());
            for (auto c : address) {
                drawChar(pos, c, Qt::black, Qt::white, p);
                pos += step;
            }
            pos += {m_dataMargin, 0.};
            auto bytes = std::min(m_dataSize - offset, (size_t)m_chars);
            for (int n = bytes; n; n--) {
                drawChar(pos, decode(m_data[m_dataStart + offset++]), Qt::red, Qt::white, p);
                pos += step;
            }
            pos = {0., pos.y() + yStep()};
        }
        newTime(time.elapsed());
    }
    void resizeEvent(QResizeEvent *) override {
        initData();
    }
    void scrollContentsBy(int, int) override {
        m_dataStart = verticalScrollBar()->value() * (size_t)m_chars;
        viewport()->update();
    }
public:
    HexView(QWidget * parent = nullptr) : HexView(nullptr, 0, parent) {}
    HexView(const char * data, size_t size, QWidget * parent = nullptr) :
        QAbstractScrollArea{parent}, m_data(data), m_dataSize(size)
    {
        const QFontMetrics fm(m_font);
        for (int i = 0x20; i < 0xE0; ++i)
            m_glyphRect = m_glyphRect.united(fm.boundingRect(CP437[i]));
        m_glyphPos = {-m_glyphRect.left(), -m_glyphRect.top()};
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
    Q_SIGNAL void newTime(int);
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
    QObject::connect(&view, &HexView::newTime, &status, QOverload<int>::of(&QLabel::setNum));
    charset.click();
    ui.resize(1000, 800);
    ui.show();
    return app.exec();
}

#include "main.moc"
