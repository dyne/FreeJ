#ifndef __DelaygrabFilter__
#define __DelaygrabFilter__

#include <filter.h>

#define MODES 4

class Delaygrab: public Filter {
private:
  int x,y,i,xyoff,v;
  //  unsigned char *imagequeue,*curqueue;
  Uint8 *imagequeue,*curqueue;
  int curqueuenum;
  Uint32 *curdelaymap;
  Uint8 *curpos,*curimage;
  int curposnum;
  void *delaymap;

  /* initialized from the init */
  int delaymapwidth;  /* width/blocksize */
  int delaymapheight; /* height/blocksize */
  int delaymapsize;   /* delaymapheight*delaymapwidth */

  int blocksize;
  int block_per_pitch;
  int block_per_bytespp;
  int block_per_res;

  int current_mode;

public:
  Delaygrab();
  ~Delaygrab() { _delete(); };

  void *process(void *buffo);
  bool init();
  void _delete();

  void createDelaymap(int mode);
  void inc_mode() { if(current_mode<MODES) createDelaymap(current_mode+1); };
  void dec_mode() { if(current_mode>1) createDelaymap(current_mode-1); };
  
  void set_blocksize(int bs);
  void inc_blocksize() { set_blocksize(blocksize+1); };
  void dec_blocksize() { if(blocksize>1) set_blocksize(blocksize-1); };

  bool kbd_input(SDL_keysym *keysym);

  void *procbuf;

};

#endif
