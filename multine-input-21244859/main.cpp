#include <QApplication>
#include <QInputDialog>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QInputDialog::getText(nullptr, "Title", "Hello World !! \nWhat goes in here");
    return 0;
}
