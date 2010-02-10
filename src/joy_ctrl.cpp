/*  FreeJ
 *  (c) Copyright 2006-2007 Denis Roio aka jaromil <jaromil@dyne.org>
 *   fixed at CCC camp 2007 by MrGoil
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

#include <string.h>

#include <joy_ctrl.h>

#include <config.h>

#include <context.h>
#include <jutils.h>

#include <jsparser.h>
#include <callbacks_js.h> // javascript
#include <jsparser_data.h>

/////// Javascript JoystickController
JS(js_joy_ctrl_constructor);

DECLARE_CLASS_GC("JoystickController",js_joy_ctrl_class, js_joy_ctrl_constructor,js_ctrl_gc);

JSFunctionSpec js_joy_ctrl_methods[] = {
#ifdef HAVE_LINUX
  {"init_rumble",    js_joy_init_rumble,     1},
  {"rumble",         js_joy_rumble,          1},
#endif
  {0}
};



JoyController::JoyController()
  :SdlController() {
    
    set_name("Joystick");
    
    num = 0;
}

JoyController::~JoyController() {
  int c;

  for(c=0;c<num;c++)
    SDL_JoystickClose(joy[c]);

}

bool JoyController::init(Context *freej) {

  //bool JoyController::init(Context *context) {
  func("JoyController::init()");
 
  int found = 0;
  int c;

  num = SDL_NumJoysticks();
  if(num>4) num = 4; // we support maximum 4 joysticks

  func("num joysticks %i",num);
  for(c=0;c<num;c++) {
    joy[found] = SDL_JoystickOpen(c);
    if(joy[found]) {
      if(strstr(SDL_JoystickName(c),"Keyboard")) {
	/* this is not a joystick! it happens on MacOSX
	   to have "Apple Extended USB Keyboard" recognized as joystick */
	SDL_JoystickClose(joy[found]);
	continue;
      }
      notice("Joystick: %s",SDL_JoystickName(c));
      axes = SDL_JoystickNumAxes(joy[found]);
      buttons = SDL_JoystickNumButtons(joy[found]);
      balls = SDL_JoystickNumBalls(joy[found]);
      hats = SDL_JoystickNumHats(joy[found]);
      act("%i axes, %i balls, %i hats, %i buttons",
	  axes, balls, hats, buttons);
      found++;
    } else {
      error("error opening %s",SDL_JoystickName(c));
    }
  }
  
  num = found;
  
  if(!num) {
    notice("no joystick found");
    return(false);
  } else
    SDL_JoystickEventState(SDL_ENABLE);
  
  func("%s",__PRETTY_FUNCTION__);
  env = freej;
  jsenv = freej->js->global_context;
  jsobj = freej->js->global_object;
  
  initialized = true;
  return(true);
  
}

int JoyController::poll() {
	poll_sdlevents(SDL_JOYEVENTMASK); // calls dispatch() 
	return 0;
}

int JoyController::axismotion(int device, int axis, int value) {
	return JSCall("axismotion", 3, "iii", device, axis, value);
}

int JoyController::ballmotion(int device, int ball, int xrel, int yrel) {
	return JSCall("ballmotion", 4, "iiii", device, ball, xrel, yrel);
}

int JoyController::hatmotion(int device, int hat, int value) {
	return JSCall("hatmotion", 3, "iii",  device, hat, value);
}

int JoyController::button_down(int device, int button) {
	return JSCall("button", 3, "iic", device, button, 1);
}

int JoyController::button_up(int device, int button) {
	return JSCall("button", 3, "iic", device, button, 0);
}

int JoyController::dispatch() {

	switch(event.type) {
		
	case SDL_JOYAXISMOTION:
		return axismotion(event.jaxis.which, event.jaxis.axis,
				  event.jaxis.value);
		break;

	case SDL_JOYBALLMOTION:
		return ballmotion(event.jball.which, event.jball.ball,
				  event.jball.xrel, event.jball.yrel);
		break;

	case SDL_JOYHATMOTION:
		return hatmotion(event.jhat.which, event.jhat.hat,
				 event.jhat.value);
		break;
		
	case SDL_JOYBUTTONDOWN:
		return button_down(event.jbutton.which, event.jbutton.button);
		break;
		
	case SDL_JOYBUTTONUP:
		return button_up(event.jbutton.which, event.jbutton.button);
		break;

	default: break;

	}

	return 0;
}


#ifdef HAVE_LINUX


#define BITS_PER_LONG (sizeof(long) * 8)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)    ((array[LONG(bit)] >> OFF(bit)) & 1)


static const char* effect_names[] = {
  (const char*)"Sine vibration",
  (const char*)"Constant Force",
  (const char*)"Spring Condition",
  (const char*)"Damping Condition",
  (const char*)"Strong Rumble",
  (const char*)"Weak Rumble"
};

bool JoyController::init_rumble(char *devfile) {
  unsigned long features[4];
  int n_effects;    /* Number of effects the device can play at the same time */
  
  /* Open device */
  rumble_fd = open(devfile, O_RDWR);
  if (rumble_fd == -1) {
    error("Cannot open rumble event device");
    return(false);
  }

  act("Joystick rumble device open");

  /* Query device */
  if (ioctl(rumble_fd, EVIOCGBIT(EV_FF, sizeof(unsigned long) * 4), features) == -1) {
    perror("Ioctl query");
    exit(1);
  }  
  if (ioctl(rumble_fd, EVIOCGEFFECTS, &n_effects) == -1) {
    error("Error on IOctl to get number of effects");
  }

  /* download a periodic sinusoidal effect */
  effects[0].type = FF_PERIODIC;
  effects[0].id = -1;
  effects[0].u.periodic.waveform = FF_SINE;
  effects[0].u.periodic.period = 0.1*0x100; /* 0.1 second */
  effects[0].u.periodic.magnitude = 0x4000; /* 0.5 * Maximum magnitude */
  effects[0].u.periodic.offset = 0;
  effects[0].u.periodic.phase = 0;
  effects[0].direction = 0x4000;      /* Along X axis */
  effects[0].u.periodic.envelope.attack_length = 0x100;
  effects[0].u.periodic.envelope.attack_level = 0;
  effects[0].u.periodic.envelope.fade_length = 0x100;
  effects[0].u.periodic.envelope.fade_level = 0;
  effects[0].trigger.button = 0;
  effects[0].trigger.interval = 0;
  effects[0].replay.length = 20000;  /* 20 seconds */
  effects[0].replay.delay = 0;
  
  if (ioctl(rumble_fd, EVIOCSFF, &effects[0]) == -1) {
    error("Error uploading effect 0");
  }
  
  /* download a constant effect */
  effects[1].type = FF_CONSTANT;
  effects[1].id = -1;
  effects[1].u.constant.level = 0x2000;     /* Strength : 25 % */
  effects[1].direction = 0x6000;      /* 135 degrees */
  effects[1].u.constant.envelope.attack_length = 0x100;
  effects[1].u.constant.envelope.attack_level = 0;
  effects[1].u.constant.envelope.fade_length = 0x100;
  effects[1].u.constant.envelope.fade_level = 0;
  effects[1].trigger.button = 0;
  effects[1].trigger.interval = 0;
  effects[1].replay.length = 20000;  /* 20 seconds */
  effects[1].replay.delay = 0;
  
  if (ioctl(rumble_fd, EVIOCSFF, &effects[1]) == -1) {
    error("Error uploading effect 1");
  }

  /* download an condition spring effect */
  effects[2].type = FF_SPRING;
  effects[2].id = -1;
  effects[2].u.condition[0].right_saturation = 0x7fff;
  effects[2].u.condition[0].left_saturation = 0x7fff;
  effects[2].u.condition[0].right_coeff = 0x2000;
  effects[2].u.condition[0].left_coeff = 0x2000;
  effects[2].u.condition[0].deadband = 0x0;
  effects[2].u.condition[0].center = 0x0;
  effects[2].u.condition[1] = effects[2].u.condition[0];
  effects[2].trigger.button = 0;
  effects[2].trigger.interval = 0;
  effects[2].replay.length = 20000;  /* 20 seconds */
  effects[2].replay.delay = 0;
  
  if (ioctl(rumble_fd, EVIOCSFF, &effects[2]) == -1) {
    error("Error uploading effect 2");
  }

  /* download an condition damper effect */
  effects[3].type = FF_DAMPER;
  effects[3].id = -1;
  effects[3].u.condition[0].right_saturation = 0x7fff;
  effects[3].u.condition[0].left_saturation = 0x7fff;
  effects[3].u.condition[0].right_coeff = 0x2000;
  effects[3].u.condition[0].left_coeff = 0x2000;
  effects[3].u.condition[0].deadband = 0x0;
  effects[3].u.condition[0].center = 0x0;
  effects[3].u.condition[1] = effects[3].u.condition[0];
  effects[3].trigger.button = 0;
  effects[3].trigger.interval = 0;
  effects[3].replay.length = 20000;  /* 20 seconds */
  effects[3].replay.delay = 0;
  
  if (ioctl(rumble_fd, EVIOCSFF, &effects[3]) == -1) {
    error("Error uploading effect 3");
  }

  /* a strong rumbling effect */
  effects[4].type = FF_RUMBLE;
  effects[4].id = -1;
  effects[4].u.rumble.strong_magnitude = 0x8000;
  effects[4].u.rumble.weak_magnitude = 0;
  effects[4].replay.length = 5000;
  effects[4].replay.delay = 1000;
  
  if (ioctl(rumble_fd, EVIOCSFF, &effects[4]) == -1) {
    error("Error uploading effect 4");
  }

  /* a weak rumbling effect */
  effects[5].type = FF_RUMBLE;
  effects[5].id = -1;
  effects[5].u.rumble.strong_magnitude = 0;
  effects[5].u.rumble.weak_magnitude = 0xc000;
  effects[5].replay.length = 5000;
  effects[5].replay.delay = 0;
  
  if (ioctl(rumble_fd, EVIOCSFF, &effects[5]) == -1) {
    error("Error uploading effect 5");
  }

  act("5 rumble effects initialised");
  return(true);
}

bool JoyController::rumble(int intensity) {

  int i;
  if(intensity <0) {
    /* Stop the effects */
    for (i=0; i<N_EFFECTS; ++i) {
      stop.type = EV_FF;
      stop.code =  effects[i].id;
      stop.value = 0;
      
      if (write(rumble_fd, (const void*) &stop, sizeof(stop)) == -1)
	error("Error stoping joystick rumble effect %u", i);

    }
    return(true);
  }

  if(intensity > N_EFFECTS) {
    error("effect %u is out of bounds",intensity);
    return(false);
  }
  
  play.type = EV_FF;
  play.code = effects[intensity].id;
  play.value = 1;
  
  if (write(rumble_fd, (const void*) &play, sizeof(play)) == -1) {
    error("Error playing joystick rumble effect %u", intensity);
    return(false);
  }

  return(true);

}
#endif




JS(js_joy_ctrl_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  char excp_msg[MAX_ERR_MSG + 1];

  JoyController *joy = new JoyController();

  // initialize with javascript context
  if(! joy->init(global_environment) ) {
    sprintf(excp_msg, "failed initializing joystick controller");
    goto error;
  }

  // assign instance into javascript object
  if( ! JS_SetPrivate(cx, obj, (void*)joy) ) {
    sprintf(excp_msg, "failed assigning joystick controller to javascript");
    goto error;
  }

  // assign the real js object
  joy->jsobj = obj;

  joy->javascript = true;

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;

error:
    JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
              JSSMSG_FJ_CANT_CREATE, __func__, excp_msg);
    //    cx->newborn[GCX_OBJECT] = NULL;
    // same omissis as in callbacks_js.h
    delete joy; return JS_FALSE;
}

#ifdef HAVE_LINUX
// joystick rumble is supported only under linux so far...

JS(js_joy_init_rumble) {
  func("%s",__PRETTY_FUNCTION__);

  JoyController *joy = (JoyController*) JS_GetPrivate(cx, obj);
  if(!joy) JS_ERROR("JOY code data is NULL");
  
  JS_CHECK_ARGC(1);

  char *devfile = js_get_string(argv[0]);

  if( joy->init_rumble(devfile) )
  act("Joystick controller opened rumble device %s", devfile);
  
  return JS_TRUE;
}

JS(js_joy_rumble) {
  func("%s", __PRETTY_FUNCTION__);

  JoyController *joy = (JoyController*) JS_GetPrivate(cx, obj);
  if(!joy) JS_ERROR("JOY code data is NULL");

  JS_CHECK_ARGC(1);

  uint16_t val;
  JS_ValueToUint16(cx, argv[0], &val);
  
  joy->rumble(val);

  return JS_TRUE;
}

#endif
