#include <QApplication>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    const QString html(
                "<html><head><title>Page Title</title>"
                "<link rel=\"stylesheet\" type=\"text/css\" href=\"styles.css\">"
                "<link rel=\"stylesheet\" type=\"text/css\" href=\"morestyles.css\">"
                "<script type=\"text/javascript\" src=\"script1.php\"> </script>"
                "<script type=\"text/javascript\" src=\"script2.php\"> </script>"
                "<script type=\"text/javascript\" src=\"script3.php\"> </script>"
                "<script type=\"text/javascript\" src=\"script4.php\"> </script>"
                "</head><body>"
                "<TABLE class=\"A\">"
                "<TR class=\"A\">"
                "<TD>some text...</TD>"
                "<TD>some text...</TD>"
                "</TR>"
                "<TR class=\"A\">"
                "<TD>some text...</TD>"
                "<TD>some text...</TD>"
                "</TR>"
                "</TABLE>"
                "</body></html>");
    QWebPage p;
    p.mainFrame()->setHtml(html);
    QWebElement we(p.mainFrame()->documentElement());
    QWebElementCollection elements(we.findAll(".A"));
    QStringList elist;
    foreach(const QWebElement &e, elements)
        elist.append(e.toOuterXml());
    qDebug() << elist;
    return 0;
}
