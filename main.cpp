#include "SerialMX4MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SerialMX4MainWindow w;
    w.show();

    return a.exec();
}
