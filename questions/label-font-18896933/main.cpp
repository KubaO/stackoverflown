#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>
#include <QDebug>

static bool isFixedPitch(const QFont & font) {
    const QFontInfo fi(font);
    qDebug() << fi.family() << fi.fixedPitch();
    return fi.fixedPitch();
}

static QFont getMonospaceFont(){
    QFont font("monospace");
    if (isFixedPitch(font)) return font;
    font.setStyleHint(QFont::Monospace);
    if (isFixedPitch(font)) return font;
    font.setStyleHint(QFont::TypeWriter);
    if (isFixedPitch(font)) return font;
    font.setFamily("courier");
    if (isFixedPitch(font)) return font;
    return font;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString text("0123456789ABCDEF");
    QWidget w;
    QVBoxLayout * l = new QVBoxLayout(&w);
    QLabel * l1 = new QLabel(text);
    l1->setFont(getMonospaceFont());
    l->addWidget(l1);
    l->addWidget(new QLabel(text));
    w.show();
    return a.exec();
}
