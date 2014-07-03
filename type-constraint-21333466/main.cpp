#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QEventLoop>

class IClever {
public:
    virtual bool supports(QStyle::ControlElement) = 0;
    static IClever * cast(QStyle * style) {
        return dynamic_cast<IClever*>(style);
    }
};

class MyStyle : public QStyle, public IClever {
    bool supports(QStyle::ControlElement el) { }
    //...
};

enum { kCE_MYShapedFrame };
QStyle::ControlElement CE_MYShapedFrame() {
    return (QStyle::ControlElement)kCE_MYShapedFrame;
}

class MyWidget : public QWidget {
    void paintEvent(QPaintEvent*) {
        QStyleOption opt;
        opt.initFrom(this);
        QPainter painter(this);
        if (IClever::cast(style()) && IClever::cast(style())->supports(CE_MYShapedFrame())) {
            style()->drawControl(CE_MYShapedFrame(), &opt, &painter);
        } else {
            style()->drawControl(QStyle::CE_ShapedFrame, &opt, &painter);
        }
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    return a.exec();
}

#if 0
#include <type_traits>

template <typename BaseStyle>
class CleverStyle : public BaseStyle {
    template <typename T> struct isGoodBase :
            std::conditional< std::is_same<QProxyStyle, T>::value, std::true_type,
            typename std::conditional< std::is_same<QCommonStyle, T>::value, std::true_type,
            typename std::conditional< std::is_same<QStyle, T>::value, std::true_type,
            std::false_type >::type >::type >::type {};
    static_assert(isGoodBase<BaseStyle>::value, "");
public:
    virtual bool supports(QStyle::ControlElement) = 0;
};
#endif


