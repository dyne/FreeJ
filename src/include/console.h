
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <SDL/SDL.h>

class Context;

class ConsoleParser {
 public:
  
  ConsoleParser();
  ~ConsoleParser();
  
  bool init(Context *freej);
  void activate();
  void deactivate();
  void read(SDL_Event *event);
  
  Context *env;
  
  char line[256];
  int curpos;
};

#endif
