
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <slw.h>
#include <slw_console.h>
#include <slw_log.h>
#include <slw_prompt.h>

#include <jutils.h>


SLangConsole con;
SLW_Log log;
SLW_Prompt prompt;

int main(int argc, char **argv) {
  int key;
  bool quit;

  assert( con.init() );

  SLtt_set_cursor_visibility(0);

  assert( con.place(&log, 1, 0, con.w, con.h-2) );
  assert( log.init() );

  assert( con.place(&prompt, 0, con.h-1, con.w, con.h) );
  assert( prompt.init() );

  con.focused = &prompt;

  quit = false;

  while(!quit) {

    key = con.getkey();

    prompt.feed(key);
    log.feed(key);

    if( ! con.refresh() ) quit = true;
  }

  con.close();

  exit(1);
}


