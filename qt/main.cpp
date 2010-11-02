#include <QtGui/QApplication>
#include "qfreej.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Qfreej w;
    w.show();

    return a.exec();
}
