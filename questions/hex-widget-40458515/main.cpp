// https://github.com/KubaO/stackoverflown/tree/master/questions/hex-widget-40458515
#include <QtWidgets>
#include <algorithm>
#include <cmath>

const QString & CP437() {
    static auto const set = QStringLiteral(
                " ☺☻♥♦♣♠•◘○◙♂♀♪♫☼▶◀↕‼¶§▬↨↑↓→←∟↔▲▼"
                "␣!\"#$%&'()*+,-./0123456789:;<=>?"
                "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                "`abcdefghijklmnopqrstuvwxyz{|}~ "
                "ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜ¢£¥₧ƒ"
                "áíóúñÑªº¿⌐¬½¼¡«»░▒▓│┤╡╢╖╕╣║╗╝╜╛┐"
                "└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀"
                "αßΓπΣσµτΦΘΩδ∞φε∩≡±≥≤⌠⌡÷≈°∙·√ⁿ²■ ");
    return set;
}

class HexView : public QAbstractScrollArea {
    const int m_addressChars = 8;
    const qreal m_dataMargin = 4.;
    const char * m_data;
    size_t m_size;
    size_t m_start = 0;
    QRectF m_glyphRect{0.,0.,1.,1.};
    QPointF m_glyphPos;
    int m_chars, m_lines;
    QMap<QChar, QImage> m_glyphs;
    QFont m_font{"Monaco"};
    qreal xStep() const { return m_glyphRect.width(); }
    qreal yStep() const { return m_glyphRect.height(); }
    static QChar decode(char ch) { return CP437()[(uchar)ch]; }
    void drawChar(const QPointF & pos, QChar ch, QColor color, QPainter & p) {
        auto & glyph = m_glyphs[ch];
        if (glyph.isNull()) {
            glyph = QImage{m_glyphRect.size().toSize(), QImage::Format_ARGB32_Premultiplied};
            glyph.fill(Qt::white);
            QPainter p{&glyph};
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.setFont(m_font);
            p.drawText(m_glyphPos, {ch});
        }
        auto rect = m_glyphRect;
        rect.moveTo(pos);
        p.fillRect(rect, color);
        p.drawImage(pos, glyph);
    }
    void initData() {
        qreal width = viewport()->width() - m_addressChars*xStep() - m_dataMargin;
        m_chars = (width > 0.) ? width/xStep() : 0.;
        m_lines = viewport()->height()/yStep();
        if (m_chars && m_lines) {
            verticalScrollBar()->setRange(0, m_size/m_chars);
            verticalScrollBar()->setValue(m_start/m_chars);
        } else {
            verticalScrollBar()->setRange(0, 0);
        }
    }
    void paintEvent(QPaintEvent *) override {
        QPainter p{viewport()};
        QPointF pos;
        QPointF step{xStep(), 0.};
        auto dividerX = m_addressChars*xStep() + m_dataMargin/2.;
        p.drawLine(dividerX, 0, dividerX, viewport()->height());
        int offset = 0;
        while (offset < m_chars*m_lines && m_start + offset < m_size) {
            auto rawAddress = QString::number(m_start + offset, 16);
            auto address = QString{m_addressChars-rawAddress.size(), ' '} + rawAddress;
            for (auto c : address) {
                drawChar(pos, c, Qt::black, p);
                pos += step;
            }
            pos += QPointF{m_dataMargin, 0.};
            auto bytes = std::min(m_size - offset, (size_t)m_chars);
            for (int n = bytes; n; n--) {
                drawChar(pos, decode(m_data[m_start + offset++]), Qt::red, p);
                pos += step;
            }
            pos = QPointF{0., pos.y() + yStep()};
        }
    }
    void resizeEvent(QResizeEvent *) override {
        initData();
    }
    void scrollContentsBy(int, int) override {
        m_start = verticalScrollBar()->value() * (size_t)m_chars;
        viewport()->update();
    }
public:
    HexView(QWidget * parent = nullptr) : HexView(nullptr, 0, parent) {}
    HexView(const char * data, size_t size, QWidget * parent = nullptr) :
        QAbstractScrollArea{parent}, m_data(data), m_size(size)
    {
        auto fm = QFontMetrics(m_font);
        for (int i = 0x20; i < 0xE0; ++i)
            m_glyphRect = m_glyphRect.united(fm.boundingRect(CP437()[i]));
        m_glyphPos = {-m_glyphRect.left(), -m_glyphRect.top()};
        initData();
    }
    void setData(const char * data, size_t size) {
        if (data == m_data && size == m_size) return;
        m_data = data;
        m_size = size;
        m_start = 0;
        initData();
        viewport()->update();
    }
};

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    QFile file{app.applicationFilePath()};
    if (!file.open(QIODevice::ReadOnly)) return 1;
    const char * const map = (char*)file.map(0, file.size(), QFile::MapPrivateOption);
    if (!map) return 2;

    QWidget ui;
    QGridLayout layout{&ui};
    HexView view;
    QRadioButton exe{"Executable"};
    QRadioButton charset{"Character Set"};
    layout.addWidget(&view, 0, 0, 1, 3);
    layout.addWidget(&exe, 1, 0);
    layout.addWidget(&charset, 1, 1);
    QObject::connect(&exe, &QPushButton::clicked, [&]{
        view.setData(map, (size_t)file.size());
    });
    QObject::connect(&charset, &QPushButton::clicked, [&]{
        static QByteArray data;
        if (data.isNull()) {
            data.resize(256);
            for (int i = 0; i < data.size(); ++i) data[i] = (char)i;
        }
        view.setData(data.constData(), (size_t)data.size());
    });
    charset.click();
    ui.show();
    return app.exec();
}
