// https://github.com/KubaO/stackoverflown/tree/master/questions/scene-radar-40680065
#include <QtWidgets>
#include <random>

QPointF randomPosition() {
    static std::random_device dev;
    static std::default_random_engine eng(dev());
    static std::uniform_real_distribution<double> posDis(-100., 100.); // NM
    return {posDis(eng), posDis(eng)};
}

class EmptyItem : public QGraphicsItem {
public:
    QRectF boundingRect() const override { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override {}
};

class SceneManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool microGraticuleVisible READ microGraticuleVisible WRITE setMicroGraticuleVisible)
    QGraphicsScene m_scene;
    QPen m_targetPen{Qt::green, 1};
    EmptyItem m_target, m_center, m_macroGraticule, m_microGraticule;
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::Resize
                && qobject_cast<QGraphicsView*>(watched))
            emit viewResized();
        return QObject::eventFilter(watched, event);
    }
public:
    SceneManager() {
        m_scene.addItem(&m_center);
        m_scene.addItem(&m_macroGraticule);
        m_scene.addItem(&m_microGraticule);
        m_scene.addItem(&m_target);
        m_targetPen.setCosmetic(true);
        addGraticules();
    }
    void monitor(QGraphicsView *view) { view->installEventFilter(this); }
    QGraphicsScene * scene() { return &m_scene; }
    Q_SLOT void setMicroGraticuleVisible(bool vis) { m_microGraticule.setVisible(vis); }
    bool microGraticuleVisible() const { return m_microGraticule.isVisible(); }
    Q_SIGNAL void viewResized();
    void newTargets(int count = 200) {
        qDeleteAll(m_target.childItems());
        for (int i = 0; i < count; ++i) {
            auto target = new QGraphicsEllipseItem(-1.5, -1.5, 3., 3., &m_target);
            target->setPos(randomPosition());
            target->setPen(m_targetPen);
            target->setBrush(m_targetPen.color());
            target->setFlags(QGraphicsItem::ItemIgnoresTransformations);
        }
    }
    void addGraticules() {
        QPen pen{Qt::white, 1};
        pen.setCosmetic(true);
        auto center = {QLineF{-5.,0.,5.,0.}, QLineF{0.,-5.,0.,5.}};
        for (auto l : center) {
            auto c = new QGraphicsLineItem{l, &m_center};
            c->setFlags(QGraphicsItem::ItemIgnoresTransformations);
            c->setPen(pen);
        }
        for (auto range = 10.; range < 101.; range += 10.) {
            auto circle = new QGraphicsEllipseItem(0.-range, 0.-range, 2.*range, 2.*range, &m_macroGraticule);
            circle->setPen(pen);
        }
        pen = QPen{Qt::white, 1, Qt::DashLine};
        pen.setCosmetic(true);
        for (auto range = 2.5; range < 9.9; range += 2.5) {
            auto circle = new QGraphicsEllipseItem(0.-range, 0.-range, 2.*range, 2.*range, &m_microGraticule);
            circle->setPen(pen);
        }
    }
};

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    SceneManager mgr;
    mgr.newTargets();

    QWidget w;
    QGridLayout layout{&w};
    QGraphicsView view;
    QComboBox combo;
    QPushButton newTargets{"New Targets"};
    layout.addWidget(&view, 0, 0, 1, 2);
    layout.addWidget(&combo, 1, 0);
    layout.addWidget(&newTargets, 1, 1);

    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setBackgroundBrush(Qt::black);
    view.setScene(mgr.scene());
    view.setRenderHint(QPainter::Antialiasing);
    mgr.monitor(&view);

    combo.addItems({"10", "25", "50", "100"});
    auto const recenterView = [&]{
        auto range = combo.currentText().toDouble();
        view.fitInView(-range, -range, 2.*range, 2.*range, Qt::KeepAspectRatio);
        mgr.setMicroGraticuleVisible(range <= 20.);
    };
    QObject::connect(&combo, &QComboBox::currentTextChanged, recenterView);
    QObject::connect(&mgr, &SceneManager::viewResized, recenterView);
    QObject::connect(&newTargets, &QPushButton::clicked, [&]{ mgr.newTargets(); });
    w.show();
    return app.exec();
}

#include "main.moc"
