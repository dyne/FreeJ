
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
  void parse();
  void print();
  
  Context *env;
  
  char line[256];
  int curpos;

  

 private:
  /* used by the font renderer */
  int y,x,i,len,f,v,ch,cv;
  uint32_t *ptr;
  uint32_t *line_offset;
  
};

#endif
