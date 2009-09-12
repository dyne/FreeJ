/*  FreeJ
 *  (c) Copyright 2009 Andrea Guzzo <xant@dyne.org>
 *
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

#import <CVScreen.h>
#import <CFreej.h>
#import <CVLayerView.h>
#import <CVScreenView.h>
#import <QuartzCore/CIKernel.h>
#import <QTKit/QTMovie.h>

FACTORY_REGISTER_INSTANTIATOR(ViewPort, CVScreen, Screen, cocoa);

CVScreen::CVScreen()
  : ViewPort() {

  view = NULL;
}

CVScreen::~CVScreen() {
  func("%s",__PRETTY_FUNCTION__);

}

bool CVScreen::_init() {

  return true;
}


void *CVScreen::coords(int x, int y) {
    //func("method coords(%i,%i) invoked", x, y);
    // if you are trying to get a cropped part of the layer
    // use the .pitch parameter for a pre-calculated stride
    // that is: number of bytes for one full line
    return ( x + (geo.w*y) + (uint32_t*)get_surface() );
}

void *CVScreen::get_surface() {
  if (view)
    return [view getSurface];
  return NULL;
}

void CVScreen::set_view(CVScreenView *v)
{
    view = v;
    [view setSizeWidth:geo.w Height:geo.h];
}

CVScreenView *CVScreen::get_view(void)
{
    return view;
}

void CVScreen::blit(Layer *lay)
{
    if (view)
        [view drawLayer:lay];
}

void CVScreen::show() {
    //do nothing
    /*
    if (view) 
        [view setNeedsDisplay:YES];
    */
}

bool CVScreen::add_layer(Layer *lay)
{    
    bool rc = ViewPort::add_layer(lay);
    if (rc)
        [view addLayer:lay];
    return rc;
}

void CVScreen::rem_layer(Layer *lay)
{
    ViewPort::rem_layer(lay);
    [view remLayer:lay];
}

void CVScreen::resize(int rw, int rh)
{
    lock();
    geo.w = rw;
    geo.h = rh;
    unlock();
}

int CVScreen::setres(int wx, int hx)
{
    resize(wx, hx);
    return 1;
}

void CVScreen::clear()
{
}
