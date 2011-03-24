/*  FreeJ
 *  (c) Copyright 2010 Denis Roio <jaromil@dyne.org>
 *
 * based on dasciicam.c (c) 2009 Dan Stowell
 * which contained some fragments of code from hasciicam (big up hasciicam folks!)
 * but mainly coded from scratch using the V4L2 docs (big up V4L2 docs folks!)
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version  of the License,
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




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
// #include <linux/videodev2.h>

#include <ccvt.h>

#include <v4l2_layer.h>

FACTORY_REGISTER_INSTANTIATOR(Layer, V4L2CamLayer, CamLayer, v4l2);

V4L2CamLayer::V4L2CamLayer() 
  :Layer() {

  buftype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  renderhop=2; // renderhop is how many frames to guzzle before rendering
  framenum=0;
  fd = 0;
  frame = NULL;
  buffers = NULL;

  set_name("V4L2");
}

V4L2CamLayer::~V4L2CamLayer() {

  if(fd) ::close(fd);
  if(buffers) ::free(buffers);
  if(frame) ::free(frame);
}

bool V4L2CamLayer::open(const char *devfile) {
  int errno;
  if (-1 == (fd = ::open(devfile,O_RDWR|O_NONBLOCK))) {
    error("error in opening video capture device: %s", devfile);
    return(false);
  } else {
    ::close(fd);
    fd = ::open(devfile,O_RDWR);
  }
  notice("Video4Linux2 device opened: %s",devfile);
  // Check that streaming is supported
  
  memset(&capability, 0, sizeof(capability));
  if(-1 == ioctl(fd, VIDIOC_QUERYCAP, &capability)) {
    error("VIDIOC_QUERYCAP %s", strerror(errno));
    return(false);
  }
  if((capability.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0){
    error("%s does not support video capture", capability.card);
    return(false);
  }
  if((capability.capabilities & V4L2_CAP_STREAMING) == 0){
    printf("%s does not support streaming data capture", capability.card);
    return(false);
  }

  // Switch to the first video input (example 1-2)
  int index=0;
  if(-1 == ioctl(fd, VIDIOC_G_INPUT, &index)) {		//gets the current video input
    error("VIDIOC_G_INPUT: %s", strerror(errno));
    return(false);
  }

  // Get info about current video input (example 1-1)
  memset(&input, 0, sizeof(input));
  input.index = index;
  if(-1 == ioctl(fd, VIDIOC_ENUMINPUT, &input)) {
    error("VIDIOC_ENUMINPUT: %s", strerror(errno));
    return(false);
  }
  act("current input: %s", input.name);
  // example 1-6
  memset(&standard, 0, sizeof(standard));
  standard.index = 0;
  while(0 == ioctl (fd, VIDIOC_ENUMSTD, &standard)){
    if(standard.id & input.std)
      act(" + standard: %s", standard.name);
    standard.index++;
  }

  //displays the format description
  memset (&fmtdesc, 0, sizeof (fmtdesc));
  fmtdesc.index = 0;
  fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  while (0 == ioctl (fd, VIDIOC_ENUM_FMT, &fmtdesc)) {
    std::cerr << "----- format description :" << fmtdesc.description << std::endl;
    fmtdesc.index++;
  }

  memset (&format, 0, sizeof (format));
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (-1 == ioctl (fd, VIDIOC_G_FMT, &format)) {
    perror ("VIDIOC_G_FMT");
    return (false);
  }

//   memset(&format, 0, sizeof format);
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  format.fmt.pix.width = 352;
  format.fmt.pix.height = 288;
  format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  format.fmt.pix.field = V4L2_FIELD_ANY;
  if(0 == ioctl(fd, VIDIOC_TRY_FMT, &format)) {
    std::cerr << "--- we should be able to setup the resolution :)" << std::endl;
    if(-1 == ioctl(fd, VIDIOC_S_FMT, &format)) {
      error("VIDIOC_G_FMT: %s", strerror(errno));
      return(false);
    }
  }

  // Need to find out (request?) specific data format (sec 1.10.1)
  memset(&format, 0, sizeof(format));
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  //noeffect   format.fmt.pix.width  = 320;
  //noeffect   format.fmt.pix.height = 240;
  if(-1 == ioctl(fd, VIDIOC_G_FMT, &format)) {
    error("VIDIOC_G_FMT: %s", strerror(errno));
    return(false);
  }
  // the format.fmt.pix member, a v4l2_pix_format, is now filled in
  act("default capture size %ux%u, format %4.4s, bytes-per-line %u", 
      format.fmt.pix.width, format.fmt.pix.height,
      (char*)&format.fmt.pix.pixelformat, format.fmt.pix.bytesperline);

  geo.init(format.fmt.pix.width, format.fmt.pix.height, 32);

  frame = malloc(geo.bytesize);
  memset (&reqbuf, 0, sizeof (reqbuf));
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  reqbuf.memory = V4L2_MEMORY_MMAP;
  reqbuf.count = 32;
  if (-1 == ioctl (fd, VIDIOC_REQBUFS, &reqbuf)) {
    if (errno == EINVAL)
      error("video capturing by mmap-streaming is not supported");
    else
      error ("VIDIOC_REQBUFS: %s", strerror(errno));
    return(false);
  }
  act("this cam supports %i buffers", reqbuf.count);
  buffers = (bufs*)calloc (reqbuf.count, sizeof (*buffers));

  if(format.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV) { // YUYV
    warning("pixel format not recognized, trying anyway as YUYV");
    warning("the system might become instable...");
  }
  
  for (unsigned int i = 0; i < reqbuf.count; i++) {
    
    memset (&buffer, 0, sizeof (buffer));
    buffer.type = reqbuf.type;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = i;
    
    if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buffer)) {
      error ("VIDIOC_QUERYBUF: %s", strerror(errno));
      return(false);
    }
    buffers[i].length = buffer.length; /* remember for munmap() */
    
    buffers[i].start = mmap (NULL, buffer.length,
			     PROT_READ | PROT_WRITE, /* recommended */
			     MAP_SHARED,             /* recommended */
			     fd, buffer.m.offset);
    
    if (MAP_FAILED == buffers[i].start) {
      /* If you do not exit here you should unmap() and free()
       *                    the buffers mapped so far. */
      error ("mmap: %s", strerror(errno));
      return(false);
    }
  }

  // OK, the memory is mapped, so let's queue up the buffers, turn on
  // streaming, and do the business

  // queue up all the buffers for the first time
  for (unsigned int i = 0; i < reqbuf.count; i++) {
    
    memset (&buffer, 0, sizeof (buffer));
    buffer.type = reqbuf.type;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = i;
    buffer.length = buffers[i].length;
    
    if (-1 == ioctl (fd, VIDIOC_QBUF, &buffer)) {
      error ("first VIDIOC_QBUF: %s", strerror(errno));
      return(false);
    }
  }

  // turn on streaming
  if(-1 == ioctl(fd, VIDIOC_STREAMON, &buftype)) {
    error("VIDIOC_STREAMON: %s", strerror(errno));
    return(false);
  }

  set_filename(devfile);
  act("%s is supported by V4L2 layer", devfile);
  opened = true;


  return true;
}

bool V4L2CamLayer::_init() {

  // format.fmt.pix.width = geo.w;
  // format.fmt.pix.height = geo.h;

  // if(-1 == ioctl(fd, VIDIOC_S_FMT, &format)) {
  //   error("VIDIOC_S_FMT: %s", strerror(errno));
  // }
  // if(-1 == ioctl(fd, VIDIOC_G_FMT, &format)) {
  //   error("VIDIOC_G_FMT: %s", strerror(errno));
  // }
  // the format.fmt.pix member, a v4l2_pix_format, is now filled in
  return(true);
}

void *V4L2CamLayer::feed() {

  // Can we have a buffer please?
  memset(&buffer, 0, sizeof buffer);
  buffer.type = (v4l2_buf_type)buftype;
  buffer.memory = V4L2_MEMORY_MMAP;
  if (-1 == ioctl (fd, VIDIOC_DQBUF, &buffer)) {
    error ("VIDIOC_DQBUF: %s", strerror(errno));
    //    return NULL;
  }

  //multiply height by 2 seems to solve the upper half screen crop
  ccvt_yuyv_bgr32(geo.w, geo.h*2, buffers[buffer.index].start, frame);



  // Thanks for lending us your buffer, you may have it back again:
  if (-1 == ioctl (fd, VIDIOC_QBUF, &buffer)) {
    error ("VIDIOC_QBUF: %s", strerror(errno));

  } 
//	sleep(0.1);


  return frame;
}

void V4L2CamLayer::close() {
  if(!opened) return;
  // If we've finished, turn off streaming
  if(-1 == ioctl(fd, VIDIOC_STREAMOFF, &buftype)) {
    error("VIDIOC_STREAMOFF: %s", errno);
  }
  
  
  /* Cleanup. */
  for (unsigned int i = 0; i < reqbuf.count; i++)
    munmap (buffers[i].start, buffers[i].length);
  

}

