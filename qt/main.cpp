#include <QtGui/QApplication>
#include "qfreej.h"

QSize viewSize;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    viewSize.setWidth(640);
    viewSize.setHeight(480);

    Qfreej w;
    w.show();

    return a.exec();
}
