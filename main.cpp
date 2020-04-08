#include "alert.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Alert w;
    w.show();

    return a.exec();
}
