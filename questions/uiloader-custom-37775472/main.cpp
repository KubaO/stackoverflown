// https://github.com/KubaO/stackoverflown/tree/master/questions/uiloader-custom-37775472
#include <QtUiTools>
#include <QtWidgets>

const char uiData[] =
    "<ui version=\"4.0\"><class>Widget</class><widget class=\"MyWidget\" name=\"Widget\">\n"
        "<property name=\"windowTitle\" ><string>Widget</string></property>\n"
        "</widget><pixmapfunction></pixmapfunction><resources/><connections/>\n"
    "</ui>";

class MyWidget : public QLabel
{
    Q_OBJECT
    bool m_closed = false;
public:
    MyWidget(QWidget* parent = 0) : QLabel("This is MyWidget", parent) {}
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
    QApplication app(argc, argv);
    QBuffer buf;
    buf.setData(uiData, sizeof(uiData));
    MyUiLoader uiLoader;
    uiLoader.pluginPaths();
    auto uiMain = qobject_cast<MyWidget*>(uiLoader.load(&buf));
    uiMain->show();
    return app.exec();
}

#include "main.moc"
