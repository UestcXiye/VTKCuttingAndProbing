#include "VTKCuttingAndProbing.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VTKCuttingAndProbing w;
    w.show();
    return a.exec();
}
