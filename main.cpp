#include "smartserialwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SmartSerialWindow w;
    w.setWindowTitle("SmartSerial - v1.2");
    w.show();

    return a.exec();
}
