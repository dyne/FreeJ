#ifndef __JOY_CTRL_H__
#define __JOY_CTRL_H__

#include <SDL.h>
#include <jsync.h>

class Context;

class JoyControl : public JSyncThread {
 public:
  JoyControl();
  ~JoyControl();

  bool init(Context *context);
  void run();
  
  bool quit;

 private:
  SDL_Joystick *joy[4];
  int num;
  int axes;
  int buttons;
  int balls;
  int hats;
  Context *env;
  void debug();
};

#endif
