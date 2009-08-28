/*  FreeJ
 *  (c) Copyright 2008 - 2009 Denis Roio <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
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
 */


#include <config.h>

#ifdef WITH_UNICAP

#include <unicap_layer.h>

#include <ccvt.h>

#include <jutils.h>


UnicapLayer::UnicapLayer()
  :Layer() {

  
  m_handle = NULL;
  m_buffer.data = NULL;
  
  swap = 0;
  rgba[0] = NULL;
  rgba[1] = NULL;
  feed_ready = NULL;

  set_name("CAM");

  // change this to UNICAP_USER_CAPTURE
  // to switch the way unicap captures..
  capture_type = UNICAP_SYSTEM_CAPTURE;
}

UnicapLayer::~UnicapLayer() {

  close();

}


static void new_frame_bgr24_cb (unicap_event_t event, unicap_handle_t handle,
			  unicap_data_buffer_t * buffer, void *usr_data) {
  UnicapLayer *lay = (UnicapLayer*)usr_data;

  func("cam callback");

  ccvt_bgr24_bgr32(lay->geo.w, lay->geo.h, buffer->data, lay->rgba[0]);

  lay->feed_ready = lay->rgba[0];

  //  lay->swap = (lay->swap+1)%2;
}


static void new_frame_yuyv_cb (unicap_event_t event, unicap_handle_t handle,
			  unicap_data_buffer_t * buffer, void *usr_data) {
  UnicapLayer *lay = (UnicapLayer*)usr_data;

  func("cam callback");

  ccvt_yuyv_bgr32(lay->geo.w, lay->geo.h, buffer->data, lay->rgba[0]);

  lay->feed_ready = lay->rgba[0];

  //  lay->swap = (lay->swap+1)%2;
}

bool UnicapLayer::open(const char *devfile) {
  bool res = false;
  int i = 0;
  int fourcc, bpp;

  unicap_format_t format_spec;
  Parameter *p;

  if(!detected) {
    error("no video devices found");
    return(false);
  }
  
  while( unicap_enumerate_devices( &m_device_spec, &m_device, i++)
	 == STATUS_SUCCESS) {

    func("checking device match \"%s\" == \"%s\"", m_device.device, devfile);

    if(strcasecmp(devfile,m_device.device)==0) { // found

      if( unicap_open(&m_handle, &m_device ) == STATUS_SUCCESS ) {
	res = true;
	break;
      } else {
	error("error opening device %s", m_device.identifier);
	return(false);
      }
      
    } else continue;

  }

  if(!res) {
      error("Unicap device %s is not found", devfile);
      return(false);
  }

  notice("Unicap device opened: %s",m_device.identifier);

  act("available video formats:");
  unicap_void_format( &format_spec );

  i=0;
  fourcc=0;
  while(SUCCESS(unicap_enumerate_formats( m_handle, &format_spec, &m_format, i++))) {
    act("%u - %s - 0x%x - %u bpp",i, m_format.identifier, m_format.fourcc, m_format.bpp);
    switch(m_format.fourcc) {

    case 0x33524742: // BGR24
      fourcc = m_format.fourcc;
      bpp = 24;
      if(capture_type==UNICAP_SYSTEM_CAPTURE) {
	unicap_register_callback (m_handle,
				  UNICAP_EVENT_NEW_FRAME,
				  (unicap_callback_t) new_frame_bgr24_cb,
				  (void*)this);
	func("registered conversion callback BGR24");
      }
      break;

      /*
    case 0x34424752: // RGB32
      fourcc = format.fourcc;
      unicap_register_callback (m_handle,
				UNICAP_EVENT_NEW_FRAME,
				(unicap_callback_t) new_frame_rgb32_cb,
				(void*)this);
      break;
      */

    case 0x56595559: // YUYV and equivalents
    case 0x32595559:
      fourcc = m_format.fourcc;
      if(capture_type==UNICAP_SYSTEM_CAPTURE) {
	unicap_register_callback (m_handle,
				  UNICAP_EVENT_NEW_FRAME,
				  (unicap_callback_t) new_frame_yuyv_cb,
				  (void*)this);
	func("registered conversion callback YUYV");
      }
      break;

    case 0x30323449:
    case 0x56555949:
      fourcc = m_format.fourcc;
      if(capture_type==UNICAP_SYSTEM_CAPTURE) {
	unicap_register_callback (m_handle,
				  UNICAP_EVENT_NEW_FRAME,
				  (unicap_callback_t) new_frame_yuyv_cb,
				  (void*)this);
	func("registered conversion callback YUYV");
      }
      break;
	
    default: break;

    }
  }

  if(!fourcc) {
    warning("no known colorspace supported - trying YUV422");
    fourcc = 0x56595559;
  }

  if( ! SUCCESS( unicap_get_format (m_handle, &m_format) ))
    error("format get failed on capture device");
  
  // list sizes
  act("%u supported sizes:", m_format.size_count);
  act("min %ux%u - max %ux%u  - stepping %ux%u",
      m_format.min_size.width, m_format.min_size.height,
      m_format.max_size.width, m_format.max_size.height,
      m_format.h_stepping,     m_format.v_stepping);
  for(i=0; i<m_format.size_count; i++)
    act("%u - %u x %u", i, 
	m_format.sizes[i].width, m_format.sizes[i].height);

  // TODO: choose closest available size
  m_format.size.width =  geo.w;
  m_format.size.height = geo.h;
  
  if(capture_type==UNICAP_SYSTEM_CAPTURE) {
    m_format.buffer_type = UNICAP_BUFFER_TYPE_SYSTEM;
  } else {
    m_format.buffer_type = UNICAP_BUFFER_TYPE_USER;
  }

  m_format.fourcc = fourcc;
  m_format.bpp = bpp;


  act("initializing at %ux%u bpp:%u fourcc:0x%x",
      m_format.size.width, m_format.size.height,
      m_format.bpp, m_format.fourcc);

  if( ! SUCCESS( unicap_set_format(m_handle, &m_format) )) {
    error("format setting failed on capture device");
    error("maybe the size is not supported by this camera");
    error("else report your model and format strings");
    return(false);
  }
      
  // allocate first yuv buffer for grabbing
  m_buffer.data = (unsigned char*)malloc(m_format.buffer_size);
  m_buffer.buffer_size = m_format.buffer_size;

  // allocate 32bit buffer for YUV -> RGBA transform
  rgba[0] = malloc(m_format.size.width * m_format.size.height * 4);
//   rgba[1] = malloc(format.size.width * format.size.height * 4);
//   feed_ready = rgba[0];


  // properties

  // Initialize a property specifier structure
  unicap_void_property( &m_property_spec );

  i=0;
  while(SUCCESS(unicap_enumerate_properties( m_handle, &m_property_spec, &m_property, i++))) {
    
    char tmp[512];
    
    unicap_get_property(m_handle,  &m_property);
    
    sprintf(tmp,"%i - %s", i, m_property.identifier);
    switch( m_property.type ) {
    case UNICAP_PROPERTY_TYPE_RANGE:
    case UNICAP_PROPERTY_TYPE_VALUE_LIST:
      {
	sprintf(tmp,"%s = %.2f", tmp, m_property.value );
	p = new Parameter(Parameter::NUMBER);
	p->set_name(m_property.identifier);
	//	size_t s = 512;
	//	unicap_describe_property(&m_property, p->description, &s);
	p->set((void*)&m_property.value);
	parameters->append(p);
      }
      break;
      
    case UNICAP_PROPERTY_TYPE_MENU:
      {
	sprintf(tmp,"%s = %s", tmp, m_property.menu_item );
	p = new Parameter(Parameter::STRING);
	p->set_name(m_property.identifier);
	//	size_t s = 512;
	//	unicap_describe_property(&m_property, p->description, &s);
	p->set((void*)&m_property.value);
	parameters->append(p);
      }
      break;

    case UNICAP_PROPERTY_TYPE_FLAGS:
      {
	sprintf(tmp,"%s =",tmp);
	unsigned int j;
	const char *flags[] =
	  { "MANUAL", "AUTO", "ONE_PUSH", "READ_OUT",
	    "ON_OFF", "READ_ONLY", "FORMAT_CHANGE" };
	for( j = 0; j < ( sizeof( flags ) / sizeof( char* ) ); j++ ) {
	  if( ( (unsigned int)m_property.flags & ( 1<<j ) ) == ( 1<<j ) ) {
	    sprintf("%s %s",tmp, (char*)flags[j] );
	  }
	}
      }
      break;

    default: break;
    }
    act("%s",tmp);
  }
  act("%u capture device properties found", i-1);
  
  if( ! SUCCESS( unicap_start_capture( m_handle ) ))
    error("start capture failed on capture device");
  else func("capture started for CAM layer");

  if(capture_type==UNICAP_USER_CAPTURE) {
    unicap_queue_buffer(m_handle, & m_buffer);
  }

  //  set_name(m_device.device);
  opened = true;
  return(res);
}

bool UnicapLayer::_init() {
  func("%s %ux%u",__PRETTY_FUNCTION__,geo.w, geo.h);

  notice("Unicap layer initialized, devices detected:");

  unicap_void_device( &m_device_spec );

  detected = 0;
  while( SUCCESS(unicap_enumerate_devices( &m_device_spec, &m_device, detected))) {
    act("%u - %s",detected,m_device.identifier);
    detected++;
  }

  parameters = new Linklist<Parameter>();
  
  return true;
}


void *UnicapLayer::feed() {
  Parameter *p;
  if(!opened) {
    error("%s : no device opened");
    return(NULL);
  }
  //  func("%s",__PRETTY_FUNCTION__);

  // update parameters that changed
  p = parameters->begin();
  while(p) {
    if(p->changed)
      unicap_set_property_value( m_handle, p->name, *(double*)p->value);
    p->changed = false;
    p = (Parameter*)p->next;
  }

  func("unicap feed() on %s (%ux%u)", name, geo.w, geo.h);

  if(capture_type==UNICAP_USER_CAPTURE) {
    unicap_data_buffer_t *res;
    unicap_wait_buffer(m_handle,&res);
    ccvt_yuyv_bgr32(geo.w, geo.h, res->data, rgba[0]);
    unicap_queue_buffer(m_handle,res);
  }
  return rgba[0];
}


void UnicapLayer::close() {
  if(!opened) return;

  int status;

  if(unicap_is_stream_locked(&m_device))
    unicap_unlock_stream(m_handle);

  if(m_handle) {
    status = unicap_stop_capture(m_handle);
    if( ! SUCCESS( status ) ) {
      error("unicap reports error in stop_capture: 0x%x", status);
      unicap_stop_capture(m_handle);
    }
    unicap_close(m_handle);
  }

  if(m_buffer.data) free(m_buffer.data);

  if(rgba[0]) free(rgba[0]);
//   if(rgba[1]) free(rgba[1]);

  opened = false;
}


#endif
