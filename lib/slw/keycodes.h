/////////////////////
// table of key codes
// please help completing it
// send your patches to dyne.org hackers

// define what's the type of each letter
#define CHAR char
#define EOL    0x0  // '\0'
#define EOT    0x0  // '\0'



#define CHAR_BLANK 0x20

#define KEY_NEWLINE 10
#define KEY_ENTER 13

#define KEY_SPACE 32

#define KEY_BACKSPACE_ASCII 8
#define KEY_BACKSPACE 275
#define KEY_BACKSPACE_APPLE 127 
#define KEY_BACKSPACE_SOMETIMES 272

#define KEY_UP 257
#define KEY_DOWN 258
#define KEY_LEFT 259
#define KEY_RIGHT 260
#define KEY_PAGE_UP 261
#define KEY_PAGE_DOWN 262
#define KEY_HOME 263
#define KEY_DELETE 275
#define KEY_TAB 9

/* unix ctrl- commandline hotkeys */
#define KEY_CTRL_A 1 // goto beginning of line
#define KEY_CTRL_B 2 // change blit
#define KEY_CTRL_D 4 // delete char
#define KEY_CTRL_E 5 // add new effect
#define KEY_CTRL_F 6 // go fullscreen
#define KEY_CTRL_G 7
#define KEY_CTRL_H_APPLE 8 // ctrl-h on apple/OSX
#define KEY_CTRL_I 9 // OSD on/off
#define KEY_CTRL_K 11 // delete until end of line
#define KEY_CTRL_L 12 // refresh screen
#define KEY_CTRL_M 13 // move layer
#define KEY_CTRL_U 21 // delete until beginning of line
#define KEY_CTRL_H 272 // help the user
#define KEY_CTRL_J 10 // javascript command
#define KEY_CTRL_O 15 // open a file in a new layer
#define KEY_CTRL_P 16
#define KEY_CTRL_Q 17
#define KEY_CTRL_R 18
#define KEY_CTRL_S 19 // start streaming (overrides scroll lock)
#define KEY_CTRL_T 20 // new layer with text
#define KEY_CTRL_V 22 // change blit value
#define KEY_CTRL_W 23 // start stream and save to file
#define KEY_CTRL_X 24
#define KEY_CTRL_Y 25

#define KEY_PLUS 43
#define KEY_MINUS 45

#define KEY_ESC 0xffff // ESC (in linux, TODO: test other OS)
