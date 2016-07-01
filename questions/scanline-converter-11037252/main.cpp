// https://github.com/KubaO/stackoverflown/tree/master/questions/scanline-converter-11037252
//main.cpp
#include <algorithm>
#include <QtGui>

typedef QVector<QPointF> PointVector;
typedef QVector<QLineF> LineVector;
typedef QList<QLineF> LineList;

// Less-Than on x coordinates of a pair of points.
static bool xLessThan(const QPointF & p1, const QPointF & p2)
{
    return p1.x() < p2.x();
}

// Less-Than on y coordinates of a pair of points given as indices into the polygon.
class YLessThan {
    const QPolygonF & p;
public:
    YLessThan(const QPolygonF & poly) : p(poly) {}
    bool operator()(int i, int j) { return p[i].y() < p[j].y(); }
};

// A list of vertex indices in the polygon, sorted ascending in their y coordinate
static QVector<int> sortedVertices(const QPolygonF & poly)
{
    Q_ASSERT(! poly.isEmpty());
    QVector<int> vertices;
    vertices.reserve(poly.size());
    for (int i = 0; i < poly.size(); ++i) { vertices << i; }
    qSort(vertices.begin(), vertices.end(), YLessThan(poly));
    return vertices;
}

// Returns point of intersection of infinite lines ref and target
static inline QPointF intersect(const QLineF & ref, const QLineF & target)
{
    QPointF p;
    target.intersect(ref, &p);
    return p;
}

// Allows accessing polygon vertices using an indirect index into a vector of indices.
class VertexAccessor {
    const QPolygonF & p;
    const QVector<int> & i;
public:
    VertexAccessor(const QPolygonF & poly, const QVector<int> & indices) :
        p(poly), i(indices) {}
    inline QPointF operator[](int ii) const {
        return p[i[ii]];
    }
    inline QPointF prev(int ii) const {
        int index = i[ii] - 1;
        if (index < 0) index += p.size();
        return p[index];
    }
    inline QPointF next(int ii) const {
        int index = i[ii] + 1;
        if (index >= p.size()) index -= p.size();
        return p[index];
    }
};

// Returns a horizontal line scanline rendering of an unconstrained polygon.
// The lines are generated on an integer grid, but this could be modified for any other grid.
static LineVector getScanlines(const QPolygonF & poly)
{
    LineVector lines;
    if (poly.isEmpty()) return lines;
    const QVector<int> indices = sortedVertices(poly);
    VertexAccessor vertex(poly, indices);
    const QRectF bound = poly.boundingRect();
    const int l = bound.left();
    const int r = bound.right();
    int ii = 0;
    int yi = qFloor(vertex[0].y());
    QList<int> active;
    PointVector points;
    forever {
        const qreal y = yi;
        const QLineF sweeper(l, y, r, y);
        // Remove vertex from the active list if both lines extending from it are above sweeper
        for (int i = 0; i < active.size(); ) {
            const int ii = active.at(i);
            // Remove vertex
            if (vertex.prev(ii).y() < y && vertex.next(ii).y() < y) {
                active.removeAt(i);
            } else {
                ++ i;
            }
        }
        // Add new vertices to the active list
        while (ii < poly.count() && vertex[ii].y() < y) {
            active << ii++;
        }
        if (ii >= poly.count() && active.isEmpty()) break;
        // Generate sorted intersection points
        points.clear();
        foreach (int ii, active) {
            const QPointF a = vertex[ii];
            QPointF b = vertex.prev(ii);
            if (b.y() >= y) {
                points << intersect(sweeper, QLineF(a, b));
            }
            b = vertex.next(ii);
            if (b.y() >= y) {
                points << intersect(sweeper, QLineF(a, b));
            }
        }
        qSort(points.begin(), points.end(), xLessThan);
        // Generate horizontal fill segments
        for (int i = 0; i < points.size() - 1; i += 2) {
            lines << QLineF(points.at(i).x(), y, points.at(i+1).x(), y);
        }
        yi++;
    };
    return lines;
}

QVector<int> scanlineScanner(const QImage & image, const LineVector & tess)
{
    QVector<int> values;
    foreach (const QLineF & line, tess) {
        for (int x = round(line.x1()); x <= round(line.x2()); ++ x) {
            values << qGray(image.pixel(x, line.y1()));
        }
    }
    return values;
}

class Ui : public QWidget
{
    Q_OBJECT
public:
    Ui() {
        QGridLayout * layout = new QGridLayout();
        reset = new QPushButton("Reset", this);
        reset->setObjectName("reset");
        outline = new QCheckBox("Outline", this);
        outline->setObjectName("outline");
        layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 0, 1, 3);
        layout->addWidget(reset, 1, 0);
        layout->addWidget(outline, 1, 1);
        layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 1, 2);
        setLayout(layout);
        QMetaObject::connectSlotsByName(this);
    }
protected slots:
    void on_reset_clicked() {
        polygon.clear();
        scanlines.clear();
        lastPoint = QPointF();
        update();
    }
    void on_outline_stateChanged() {
        update();
    }

protected:
    void paintEvent(QPaintEvent *)
    {
        QPainter p(this);
        if (0) p.setRenderHint(QPainter::Antialiasing);
        p.setPen("cadetblue");
        if (!polygon.isEmpty() && scanlines.isEmpty()) {
            scanlines = getScanlines(polygon);
            qDebug() << "new scanlines";
        }
        p.drawLines(scanlines);
        if (outline->isChecked()) {
            p.setPen("orangered");
            p.setBrush(Qt::NoBrush);
            p.drawPolygon(polygon);
        }
        if (!lastPoint.isNull()) {
            p.setPen("navy");
            p.drawEllipse(lastPoint, 3, 3);
        }
    }
    void mousePressEvent(QMouseEvent * ev)
    {
        lastPoint = ev->posF();
        polygon << ev->posF();
        scanlines.clear();
        update();
    }
    QPointF lastPoint;
    QPolygonF polygon;
    LineVector scanlines;
    QPushButton * reset;
    QCheckBox * outline;
};

int main(int argc, char** argv)
{
    QApplication a(argc, argv);
    Ui ui;
    ui.show();
    return a.exec();
}

#include "main.moc"
