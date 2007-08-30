/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2003 Denis Rojo aka jaromil <jaromil@dyne.org>
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
 *
 * "$Id$"
 *
 */

#include <config.h>
#ifdef CONFIG_OGGTHEORA_ENCODER

#include <shouter.h>
#include <jutils.h>

extern bool got_sigpipe;

Shouter::Shouter()
  : Entry() {

  /*
    int vermaj,vermin,verpat;
    shout_version(&vermaj,&vermin,&verpat);
    func("Shouter::Shouter() using libshout version %i.%i.%i",vermaj,vermin,verpat);
  */

  // shout_init();

  ice = shout_new();
  
  //  to do this, more of the code must be changed
  //  shout_set_nonblocking(ice,1);
  
  running = false;
  retry = 0;
  errors = 0;
  
  /* setup defaults */
  // host("dyne.org");
  host("localhost");
  // ip("127.0.0.1");
  port(8000);
  // port(9000);
  user("source");
  pass("hackme");
  mount("freej.ogg");
  login(SHOUT_PROTOCOL_HTTP); // defaults to icecast 2 login now
  name("Streaming with FreeJ");
  url("http://freej.dyne.org");
  desc("Free the veejay in you");
  //  bps("24000");
  //  freq("22050");
  //  channels("1");
  
  profile_changed = true;

}

Shouter::~Shouter() {
  func("Shouter::~Shouter");
  stop();
  shout_free(ice);
  shout_shutdown();
}

bool Shouter::start() {
  if (running)
    return true;
  int res;
  char srv[64];
  apply_profile();
  pass("hackme");
  switch(login()) {
  case SHOUT_PROTOCOL_HTTP: sprintf(srv,"icecast2"); break;
  case SHOUT_PROTOCOL_ICY: sprintf(srv,"shoutcast"); break;
  default: sprintf(srv,"icecast 2"); break;
  }
  
  /*
    if(shout_get_connected(ice)) {
    // if allready connected, reconnect
    func("icecast still connected: disconnecting");
    shout_close(ice);
    shout_sync(ice);
    }
  */
  act("starting stream to %s server %s on port %u",srv,host(),port());
  
  res = shout_open(ice);
  func("Shouter::start() shout_open returns %i",res);
  //	shout_sync(ice);
  
  if(res==SHOUTERR_SUCCESS) {
    notice("Video streaming on %s",streamurl);
    running = true;
  } 
  else {
    notice ("Can't stream on %s :( probably the server is down or the parameters are wrong", streamurl);
    notice ("Check http://lab.dyne.org/FreejStreaming for more info or contine enjoying freej ;)");
    func ("shout_open: %s",shout_get_error(ice));
    //		shout_close(ice);
    //		shout_sync(ice);
    running = false;
  }
  
  return(running);
}

bool Shouter::stop() {

  if(running) {
    notice("Stopping video stream to %s",streamurl);
    shout_close(ice);
    shout_sync(ice);
    running = false;
  }
  act("stream is stopped");
  return true;
  
}

bool Shouter::apply_profile() {
  //  func("Shouter::apply_profile() on shouter id %i",id);
  
  bool res = true;
  //	if(was_running) stop();
  
  if(shout_set_host(ice,host()))
    error("shout_set_host: %s",shout_get_error(ice));
  
  if( shout_set_protocol(ice,SHOUT_PROTOCOL_HTTP) )
    error("shout_set_protocol %i: %s",login(),shout_get_error(ice));
  
  if( shout_set_port(ice,port()) )
    error("shout_set_port: %s",shout_get_error(ice));
  
  if( shout_set_password(ice,pass()) )
    error("shout_set_password: %s",shout_get_error(ice));
  else
    func("shout_set_password: %s",pass());
  
  // === fixes the format of the mountpoint
  if((mount())[0]!='/') {
    char tmp[MAX_VALUE_SIZE];
    sprintf(tmp,"/%s",mount());
    mount(tmp);
  }
  
  // use a .ogg termination
  if(!strstr(mount(),".ogg")) {
    char tmp[MAX_VALUE_SIZE];
    sprintf(tmp,"%s.ogg",mount());
    mount(tmp);
  }
  
  
  if( shout_set_mount(ice,mount()) )
    error("shout_set_mount: %s",shout_get_error(ice));
  
  if( shout_set_user(ice, user()) )
    error("shout_set_user: %s",shout_get_error(ice));
  else
    func("shout_set_user: %s",user());
  
  if( shout_set_format(ice,SHOUT_FORMAT_OGG) )
    error("shout_set_format: %s",shout_get_error(ice));
  
  if( shout_set_name(ice,name()) )
    error("shout_set_name: %s",shout_get_error(ice));
  
  if( shout_set_url(ice,url()) )
    error("shout_set_url: %s",shout_get_error(ice));
  
  if( shout_set_description(ice,desc()) )
    error("shout_set_description: %s",shout_get_error(ice));
  
  
  //if( shout_set_bitrate(ice,_bps) )
  //  error("shout_set_bitrate: %s",shout_get_error(ice));
  { 
  char temp[256];
  snprintf(temp,256,"%s ver. %s",PACKAGE,VERSION);
  if( shout_set_agent(ice,temp) )
    error("shout_set_agent: %s",shout_get_error(ice));
  }
  
  snprintf(streamurl,MAX_VALUE_SIZE,
	   "http://%s:%i%s",host(),port(),mount());
  
  //	if(was_running) { res = start(); }
  profile_changed = false;
  return res; 
}

int Shouter::send(unsigned char *buf, unsigned int enc) {
  int res = 0;
  if(!running) return(0);
  if(enc<1) return res;
  
  //	shout_sync(ice);
  
  res = shout_send(ice,(unsigned char*) buf, enc);
  if(res) {
    error("shout_send: %s",shout_get_error(ice));
    if (got_sigpipe && (res==SHOUTERR_SOCKET)) {
      errors++;
      got_sigpipe = false;
      if(errors>10) {
	res = -2;
	errors = 0;
      }
    } else res = -1;
  } else errors = 0;
  
  return(res);
}

#endif
