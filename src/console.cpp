
#include <font_acorn_8x8.h>
#include <context.h>
#include <osd.h>
#include <config.h>

ConsoleParser::ConsoleParser() {
  env=NULL;
}

ConsoleParser::~ConsoleParser() {
}

bool ConsoleParser::init(Context *freej) {
  env = freej;
  line_offset = (uint32_t*)env->screen->coords(5,env->screen->h-10);
  return true;
}

void ConsoleParser::activate() {
  SDL_EnableUNICODE(1);
  //  show_osd(": ");
  curpos = 0;
  memset(line,0,256);
}

void ConsoleParser::deactivate() {
  SDL_EnableUNICODE(0);
  //  show_osd(" ");
  curpos = 0;
}

/* activate console input */
void ConsoleParser::read(SDL_Event *event) {
  if(event->key.keysym.sym == SDLK_RETURN) {
    parse();
    deactivate();
    env->kbd.console = false;
    return;
  }

  line[curpos] = event->key.keysym.unicode;
  curpos++;
  //  snprintf(line,100,"%s%s",line,(char*)&event->key.keysym.unicode);
  //  show_osd(": %s",line);
  print();
}  
  
void ConsoleParser::parse() {
  notice("console command: %s",line);
}

void ConsoleParser::print() {
  ptr = line_offset;
  unsigned char c;
  for (x=0; line[x]!='\0'; x++) {
    for (y=0; y<CHAR_HEIGHT; y++) {
      c = fontdata[line[x] * CHAR_HEIGHT + y];
      for (i = CHAR_WIDTH; i > 0; i--) {
	if (c & (CHAR_START << i))
	  *ptr = 0xffffffff;
	ptr++;
      }
      ptr += (env->screen->w-CHAR_WIDTH);
    }
    ptr = line_offset + ((x+1)*CHAR_WIDTH);
  }
}
