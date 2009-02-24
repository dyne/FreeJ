
#include <config.h>

#include <jutils.h>
#include <blitter.h>
#include <SDL_imageFilter.h>

/// Past-frame blits

BLIT past_add(void *src, void *past, void *dst, int bytes) {
  SDL_imageFilterAdd((unsigned char*)src,
		     (unsigned char*)past,
		     (unsigned char*)dst,bytes);
}

BLIT past_addneg(void *src, void *past, void *dst, int bytes) {
  SDL_imageFilterAdd((unsigned char*)src,
		     (unsigned char*)past,
		     (unsigned char*)dst,bytes);
  SDL_imageFilterBitNegation((unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT past_absdiff(void *src, void *past, void *dst, int bytes) {
  SDL_imageFilterAbsDiff((unsigned char*)src,
			 (unsigned char*)past,
			 (unsigned char*)dst,bytes);
}

void setup_past_blits(Blitter *blitter) {
  Blit *b;

  // PAST blits
  b = new Blit(); b->set_name("PAST_ADD");
  sprintf(b->desc,"add to past frame");
  b->type = Blit::PAST;
  b->past_fun = past_add; blitlist.prepend(b);
  
  b = new Blit(); b->set_name("PAST_ADDNEG");
  sprintf(b->desc,"add to past frame and negate");
  b->type = Blit::PAST;
  b->past_fun = past_addneg; blitlist.prepend(b);

  b = new Blit(); b->set_name("PAST_ABSDIFF");
  sprintf(b->desc,"absolute difference on past frame");
  b->type = Blit::PAST;
  b->past_fun = past_absdiff; blitlist.prepend(b);
}
