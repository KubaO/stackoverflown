#include "widget.h"
#include "ui_widget.h"
#include <QIcon>
#include <QMargins>

    Widget::Widget(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::Widget)
    {
        ui->setupUi(this);
        QPushButton * btn = new QPushButton(tr("PDF Generieren..."));
        btn->setIcon(QIcon(":/images/images/arrow-right.png"));
        ui->buttonBox->addButton(btn, QDialogButtonBox::AcceptRole);
    #if (QT_VERSION <= QT_VERSION_CHECK(5, 1, 1)) && defined(QT_OS_MAC)
        QMargins margins = layout()->contentsMargins();
        margins.setBottom(margins.bottom() - 6);
        layout()->setContentsMargins(margins);
    #endif
    }

Widget::~Widget()
{
    delete ui;
}
