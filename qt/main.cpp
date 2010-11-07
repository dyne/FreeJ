#include <QtGui/QApplication>
#include "qfreej.h"

QSize viewSize;

//todo
//insert color and bool filter parameters
//put gpl titles
//be able to record the viewport, and replay it.
//replace slow button by a slider
//find a place where to the zoom x and y ratio of the layer
//devide the fake size window by two .... not sure :)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    viewSize.setWidth(640);
    viewSize.setHeight(480);

    Qfreej w;
    w.show();

    return a.exec();
}
