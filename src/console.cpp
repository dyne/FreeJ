
#include <context.h>
#include <osd.h>

ConsoleParser::ConsoleParser() {
}

ConsoleParser::~ConsoleParser() {
}

bool ConsoleParser::init(Context *freej) {
  env = freej;
  return true;
}

void ConsoleParser::activate() {
  SDL_EnableUNICODE(1);
  show_osd(": ");
  curpos = 0;
  memset(line,0,256);
}

void ConsoleParser::deactivate() {
  SDL_EnableUNICODE(0);
  show_osd(" ");
  curpos = 0;
}

/* activate console input */
void ConsoleParser::read(SDL_Event *event) {
  if(event->key.keysym.sym == SDLK_RETURN) {
    //    parse(event);
    deactivate();
    env->kbd.console = false;
    return;
  }

  line[curpos] = event->key.keysym.unicode;
  curpos++;
  //  snprintf(line,100,"%s%s",line,(char*)&event->key.keysym.unicode);
  show_osd(": %s",line);
}  
  
