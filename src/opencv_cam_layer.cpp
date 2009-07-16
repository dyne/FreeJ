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
  rgb = NULL;

  set_name("CAM");
}

OpenCVCamLayer::~OpenCVCamLayer() {

  if(rgba) cvReleaseImage(&rgba);
  if(rgb) cvReleaseImage(&rgb);
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
  func("%s %ux%u",__PRETTY_FUNCTION__,width, height);

  capture = cvCaptureFromCAM( CV_CAP_ANY );
  if( !capture ) {
    error("OpenCV capture is NULL");
    return(false);
  }
  // doesn't work, why?
  cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, width);
  cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, height);
  

  frame = cvQueryFrame( capture );
  act("Camera capture initialized: %u chans, %u depth",
      frame->nChannels, frame->depth);

  int w = (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH);
  int h = (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT);
  cvsize = cvSize(w, h);
  
  //  dst = cvCreateImage(cvsize, IPL_DEPTH_32S, 4);

  rgb = cvCreateImage(cvsize, IPL_DEPTH_32S, frame->nChannels);
  rgba = cvCreateImage(cvsize, IPL_DEPTH_32S, 4);
  
  _init(w, h);

  opened = true;
  return(true);

}

void *OpenCVCamLayer::feed() {


  IplImage *tmp;
  frame = cvQueryFrame( capture );

  cvConvert(frame, rgb);
  //  func("cvConvert from %u to %u depth", frame->depth, rgb->depth);
  cvCvtColor(rgb, rgba, CV_RGB2BGRA);
  //  func("cvCvtcolor..");
  //  cvCvtColor(tmp, dst, CV_YCrCb2RGB);

  //  cvReleaseImage( &tmp );

  return(rgba->imageData);

}

void OpenCVCamLayer::close() {

}
