#include <QApplication>
#include <QWidget>
#include <QUiLoader>
#include <QBuffer>
#include <QLabel>

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
#define Q_DECL_OVERRIDE override
#endif

const char uiData[] =
    "<ui version=\"4.0\"><class>Widget</class><widget class=\"MyWidget\" name=\"Widget\" >"
        "<property name=\"windowTitle\" ><string>Widget</string></property>"
        "</widget><pixmapfunction></pixmapfunction><resources/><connections/>\n"
    "</ui>\n";

class MyWidget : public QWidget
{
    Q_OBJECT
    bool m_closed;
public:
    MyWidget(QWidget* parent = 0) : QWidget(parent), m_closed(false) {
        new QLabel("This is MyWidget", this);
    }
    bool isClosed() const { return m_closed; }
    void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE { m_closed = true; }
};

class MyUiLoader : public QUiLoader {
public:
    MyUiLoader(QObject * parent = 0) : QUiLoader(parent) {}
    QWidget * createWidget(const QString & className, QWidget * parent = 0,
                           const QString & name = QString()) {
        if (className == "MyWidget") {
            MyWidget * w = new MyWidget(parent);
            w->setObjectName(name);
            return w;
        }
        return QUiLoader::createWidget(className, parent, name);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QBuffer buf;
    buf.setData(uiData, sizeof(uiData));
    MyUiLoader uiLoader;
    MyWidget* uiMain = qobject_cast<MyWidget*>(uiLoader.load(&buf));
    uiMain->show();
    return a.exec();
}

#include "main.moc"
