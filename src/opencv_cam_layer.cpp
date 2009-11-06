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

#ifdef WITH_OPENCV

#include <cv.h>
#include <highgui.h>

#include <opencv_cam_layer.h>

#include <ccvt.h>

#include <jutils.h>

FACTORY_REGISTER_INSTANTIATOR(Layer, OpenCVCamLayer, CamLayer, opencv);

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
  func("%s",__PRETTY_FUNCTION__);

  // examine the last cypher of the device, should be a number
  int len = strlen(devfile);
  int dev = atoi(&devfile[len-1]);

  act("opening camera device %i (%s)",dev,devfile);

  capture = cvCaptureFromCAM( dev );
  if( !capture ) {
    error("OpenCV cannot open device %i (%s)", dev, devfile);
    return(false);
  }

  // set size
  cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, geo.w);
  cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, geo.h);

  
  frame = cvQueryFrame( capture );
  func("first cvQueryFrame returns %p",frame);

  int w = (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH);
  int h = (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT);

  cvsize = cvSize(w, h);
  geo.init(w, h, 32);

  act("Camera capture initialized: %ux%u %u chans, %u depth, fourcc %s (seq %s)",
      w,h, frame->nChannels, frame->depth, frame->colorModel, frame->channelSeq);

  rgba = malloc(geo.bytesize);

  opened = true;
  return(true);
}

bool OpenCVCamLayer::_init() {
  return(true);
}

void *OpenCVCamLayer::feed() {

  frame = cvQueryFrame( capture );

  ccvt_bgr24_bgr32(geo.w, geo.h, frame->imageData, rgba);

  return(rgba);

}

void OpenCVCamLayer::close() {

}

#endif // WITH_OPENCV
