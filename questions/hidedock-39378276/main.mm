// https://github.com/KubaO/stackoverflown/tree/master/questions/hidedock-39378276
#include <QtWidgets>
#include <AppKit/AppKit.h>

void hideDockIcon() {
    [NSApp setActivationPolicy: NSApplicationActivationPolicyProhibited];
}

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    QLabel label{"Quit Me"};
    label.setMinimumSize(200, 100);
    label.show();
    int rc = app.exec();
    hideDockIcon();
    qDebug() << "cleaning up";
    QThread::sleep(5);
    qDebug() << "cleanup finished";
    return rc;
}
