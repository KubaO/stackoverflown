// https://github.com/KubaO/stackoverflown/tree/master/questions/text-wildcard-40235510
#include <QtWidgets>

constexpr auto US = QChar{0x1F};
constexpr auto RS = QChar{0x1E};

class SubstitutingStyle : public QProxyStyle
{
    Q_OBJECT
    QMap<QString, QString> substs;
public:
    static QString _(QString str) {
        str.prepend(US);
        str.append(RS);
        return str;
    }
    void add(const QString & from, const QString & to) {
        substs.insert(_(from), to);
    }
    QString substituted(QString text) const {
        for (auto it = substs.begin(); it != substs.end(); ++it)
            text.replace(it.key(), it.value());
        return text;
    }
    virtual void drawItemText(
            QPainter * painter, const QRect & rect, int flags, const QPalette & pal,
            bool enabled, const QString & text, QPalette::ColorRole textRole = QPalette::NoRole) const override;
};

void SubstitutingStyle::drawItemText(
        QPainter * painter, const QRect & rect, int flags, const QPalette & pal,
        bool enabled, const QString & text, QPalette::ColorRole textRole) const
{
    QProxyStyle::drawItemText(painter, rect, flags, pal, enabled, substituted(text), textRole);
}

template <typename Base> class SubstitutingApp : public Base {
public:
    using Base::Base;
    bool notify(QObject * obj, QEvent * ev) override {
        if (ev->type() == QEvent::WindowTitleChange) {
            auto w = qobject_cast<QWidget*>(obj);
            auto s = qobject_cast<SubstitutingStyle*>(this->style());
            if (w && s) w->setWindowTitle(s->substituted(w->windowTitle()));
        }
        return Base::notify(obj, ev);
    }
};

int main(int argc, char ** argv) {
    SubstitutingApp<QApplication> app{argc, argv};
    auto style = new SubstitutingStyle;
    app.setApplicationVersion("0.0.1");
    app.setStyle(style);
    style->add("version", app.applicationVersion());

    QLabel label{"My Version is: \x1Fversion\x1E"};
    label.setWindowTitle("Foo \x1Fversion\x1E");
    label.setMinimumSize(200, 100);
    label.show();
    return app.exec();
}
#include "main.moc"
