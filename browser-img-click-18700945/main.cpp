#include <QApplication>
#include <QTextBrowser>
#include <QImage>
#include <QPainter>
#include <QMenu>
#include <QContextMenuEvent>
#include <QTextBlock>
#include <QPointer>
#include <QDebug>

class Browser : public QTextBrowser
{
    QPointer<QMenu> m_menu;
protected:
    void contextMenuEvent(QContextMenuEvent *ev) {
        Q_ASSERT(m_menu.isNull()); // make sure the menus aren't leaking
        m_menu = createStandardContextMenu();
        QTextCursor cur = cursorForPosition(ev->pos());
        QTextCharFormat fmt = cur.charFormat();
        qDebug() << "position in block" << cur.positionInBlock()
                 << "object type" << cur.charFormat().objectType();
        if (fmt.objectType() == QTextFormat::NoObject) {
            // workaround, sometimes the cursor will point one object to the left of the image
            cur.movePosition(QTextCursor::NextCharacter);
            fmt = cur.charFormat();
        }
        if (fmt.isImageFormat()) {
            QTextImageFormat ifmt = fmt.toImageFormat();
            m_menu->addAction(QString("Image URL: %1").arg(ifmt.name()));
        }
        m_menu->move(ev->globalPos());
        m_menu->setAttribute(Qt::WA_DeleteOnClose); // the menu won't leak
        m_menu->show(); // show the menu asynchronously so as not to block the application
    }
};

void addImage(QTextDocument * doc, const QString & url) {
    QImage img(100, 100, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    p.drawRect(0, 0, 99, 99);
    p.drawText(img.rect(), url);
    doc->addResource(QTextDocument::ImageResource, QUrl(url), img);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextDocument doc;
    Browser browser;
    doc.setHtml("<img src=\"data://image1\"/><br/><img src=\"data://image2\"/>");
    addImage(&doc, "data://image1");
    addImage(&doc, "data://image2");
    browser.show();
    browser.setDocument(&doc);
    return a.exec();
}
