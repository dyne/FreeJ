/*  Qfreej
 *  (c) Copyright 2010 fred_99
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
//move main window in accordance with the size
//fix the rotation sense when changing mouse direction.
//be able to record the viewport, and replay it.
//replace slow button by a slider
//try to open some sound
//add a button in the textlayer to unfold text
//do a streaming interface
//see to use Linklist.completion to add blit, filters and generators
//devide the fake size window by two .... not sure :)
//insert STRING filters parameters


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    viewSize.setWidth(400);
    viewSize.setHeight(300);

    Qfreej w;
    w.show();

    return a.exec();
}
