/*  FreeJ
 *  (c) Copyright 2009 Denis Roio <jaromil@dyne.org>
 *
 * This source code  is free software; you can  redistribute it and/or
 * modify it under the terms of the GNU Public License as published by
 * the Free Software  Foundation; either version 3 of  the License, or
 * (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but  WITHOUT ANY  WARRANTY; without  even the  implied  warranty of
 * MERCHANTABILITY or FITNESS FOR  A PARTICULAR PURPOSE.  Please refer
 * to the GNU Public License for more details.
 *
 * You should  have received  a copy of  the GNU Public  License along
 * with this source code; if  not, write to: Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>

#include <cv.h>
#include <highgui.h>

#include <opencv_cam_layer.h>

#include <ccvt.h>

#include <jutils.h>

OpenCVCamLayer::OpenCVCamLayer()
  :Layer() {

  capture = NULL;
  frame = NULL;
  rgba = NULL;

  set_name("CAM");
}

OpenCVCamLayer::~OpenCVCamLayer() {

  if(rgba) free(rgba);
  if(frame) cvReleaseImage(&frame);
  if(capture)
    cvReleaseCapture( &capture );

}

bool OpenCVCamLayer::open(const char *devfile) {
  opened = true;
  return(true);
}

bool OpenCVCamLayer::init(Context *freej) {
  return init(freej, freej->screen->w, freej->screen->h);
}

bool OpenCVCamLayer::init(Context *freej, int width, int height) {
  func("%s",__PRETTY_FUNCTION__);

  // set size doesn't work, why?
  cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, width);
  cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, height);

  capture = cvCaptureFromCAM( CV_CAP_ANY );
  if( !capture ) {
    error("OpenCV capture is NULL");
    return(false);
  }
  
  frame = cvQueryFrame( capture );

  int w = (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH);
  int h = (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT);

  cvsize = cvSize(w, h);
  _init(w, h);

  act("Camera capture initialized: %u chans, %u depth, fourcc %s (seq %s)",
      frame->nChannels, frame->depth, frame->colorModel, frame->channelSeq);

  rgba = jalloc(geo.size);

  opened = true;
  return(true);

}

void *OpenCVCamLayer::feed() {

  frame = cvQueryFrame( capture );

  ccvt_bgr24_bgr32(geo.w, geo.h, frame->imageData, rgba);

  return(rgba);

}

void OpenCVCamLayer::close() {

}
