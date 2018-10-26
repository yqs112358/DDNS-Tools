#include "dialog.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("DDNS-AskClient");
    QCoreApplication::setApplicationVersion("1.1");
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    Dialog w;
    w.show();

    return a.exec();
}
