#include <QApplication>
#include <QAbstractNativeEventFilter>
#include <QTextStream>
#include <QWidget>
#include <cstdio>
#import <AppKit/AppKit.h>

QTextStream out(stdout);

class MyEventFilter : public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
        Q_UNUSED(eventType) Q_UNUSED(result)
        NSEvent * event = (NSEvent*)message;
        switch ([event type]) {
        case NSLeftMouseDown:
            out << "Lv"; break;
        case NSLeftMouseUp:
            out << "L^"; break;
        case NSRightMouseDown:
            out << "Rv"; break;
        case NSRightMouseUp:
            out << "R^"; break;
        case NSOtherMouseDown:
            out << [event buttonNumber] << "v"; break;
        case NSOtherMouseUp:
            out << [event buttonNumber] << "^"; break;
        default:
            return false;
        }
        out << endl;
        return false;
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSharedPointer<QAbstractNativeEventFilter> filter(new MyEventFilter);
    const int mask =
            NSLeftMouseDownMask | NSLeftMouseUpMask |
            NSRightMouseDownMask | NSRightMouseUpMask |
            NSOtherMouseDownMask | NSOtherMouseUpMask;
    // The global monitoring handler is *not* called for events sent to our application
    id monitorId = [NSEvent addGlobalMonitorForEventsMatchingMask:mask handler:^(NSEvent* event) {
        filter->nativeEventFilter("NSEvent", event, 0);
    }];
    // We also need to handle events coming to our application
    a.installNativeEventFilter(filter.data());
    QWidget w;
    w.show();
    int rc = a.exec();
    [NSEvent removeMonitor:monitorId];
    return rc;
}
