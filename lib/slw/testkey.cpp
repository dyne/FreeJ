
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <slw.h>
#include <slw_console.h>

#include <jutils.h>


class TestKey : public SLangWidget {
  
public:
  TestKey() : SLangWidget() { set_name("testkey"); };
  ~TestKey() { };

  bool init() {
    if(!console) return false;
    // default widget behaviour
    initialized = true;
    visible = true;
    cursor = false;
    can_focus = true;
    border = false;

    refresh();
    return true;
  }

  // this function gets called every time a key is passed to the widget
  // so here we draw and change the displayed information accordingly
  bool feed(int key) {
    char msg[256];
    int len;
    if(key) { // if it's not null
      snprintf(msg, 256, "[ %c : %u ]                   ", key, key);
      len = strlen(msg);
      putnch((CHAR*)&msg, 0, 0, len);
    }
    return true;
  }

  bool refresh() {
    return true;
  }

};

SLangConsole con;
TestKey test;

int main(int argc, char **argv) {
  int key;
  bool quit;

  assert( con.init() );

  SLtt_set_cursor_visibility(0);

  assert( con.place(&test, (con.w/2)-5, (con.h/2)-5 ,con.w,con.h) );
  assert( test.init() );

  con.focused = &test;

  quit = false;

  while(!quit) {

    key = con.getkey();

    test.feed(key);

    if( ! con.refresh() ) quit = true;
  }

  con.close();

  exit(1);
}


