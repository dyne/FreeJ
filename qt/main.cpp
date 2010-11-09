/*  Qfreej
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#include <QtGui/QApplication>
#include "qfreej.h"

QSize viewSize;

//todo
//finish delete track in QqFiltersApplied and specialeventget
//insert other filters parameters
//put gpl titles
//be able to record the viewport, and replay it.
//replace slow button by a slider
//find a place where to the zoom x and y ratio of the layer
//devide the fake size window by two .... not sure :)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    viewSize.setWidth(400);
    viewSize.setHeight(300);

    Qfreej w;
    w.show();

    return a.exec();
}
