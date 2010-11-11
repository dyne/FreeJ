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


#include <FakeWindow.h>


//layer
FakeWindow::FakeWindow(Context *context, Layer *layer, Geometry *geo, QWidget* parent) : QWidget(parent)
{
    m_angle=0;
    m_painter = new QPainter(this);
    qContext = context;
    qLayer = layer;
    qTextLayer = NULL;
    winGeo = new QRect(geo->x, geo->y, (int)geo->w, (int)geo->h);
    setGeometry(*winGeo);
}

//textlayer
FakeWindow::FakeWindow(Context *context, TextLayer *textlayer, Geometry *geo, QWidget *parent) : QWidget(parent)
{
    m_angle=0;
    m_painter = new QPainter(this);
    qContext = context;
    qTextLayer = textlayer;
    qLayer = NULL;
    winGeo = new QRect(geo->x, geo->y, (int)geo->w, (int)geo->h);
    setGeometry(*winGeo);
}

FakeWindow::~FakeWindow()
{
    delete m_painter;
    delete winGeo;
}

Context* FakeWindow::getContext()
{
    return(qContext);
}

Layer* FakeWindow::getLayer()
{
    return(qLayer);
}

TextLayer* FakeWindow::getTextLayer()
{
    return(qTextLayer);
}

QRect* FakeWindow::getWinGeo()
{
    return(winGeo);
}

void FakeWindow::setAngle(int angle)
{
    m_angle = angle;
}

int FakeWindow::getAngle()
{
    return(m_angle);
}

QPainter* FakeWindow::getPainter()
{
    return(m_painter);
}


