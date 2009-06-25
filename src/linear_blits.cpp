
#include <config.h>
#include <stdlib.h>


#include <jutils.h>
#include <blitter.h>

#include <SDL_imageFilter.h>



// Linear transparent blits
BLIT blit_xor(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  register int c;
  register uint32_t *s = (uint32_t*)src;
  register uint32_t *d = (uint32_t*)dst;

  for(c=bytes>>2;c>0;c--,s++,d++)
    *d ^= *s;
}


BLIT rgb_jmemcpy(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  jmemcpy(dst,src,bytes);
}

BLIT red_channel(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  register int c;
  register uint8_t *s = (uint8_t*)src;
  register uint8_t *d = (uint8_t*)dst;
  for(c=bytes>>2;c>0;c--,s+=4,d+=4)
    *(d+rchan) = *(s+rchan);
}

BLIT green_channel(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  register int c;
  register uint8_t *s = (uint8_t*)src;
  register uint8_t *d = (uint8_t*)dst;
  for(c=bytes>>2;c>0;c--,s+=4,d+=4)
    *(d+gchan) = *(s+gchan);
}

BLIT blue_channel(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  register int c;
  register uint8_t *s = (uint8_t*)src;
  register uint8_t *d = (uint8_t*)dst;
  for(c=bytes>>2;c>0;c--,s+=4,d+=4)
    *(d+bchan) = *(s+bchan);
}

BLIT red_mask(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  register int c;
  register uint32_t *s = (uint32_t*)src;
  register uint32_t *d = (uint32_t*)dst;

  for(c=bytes>>2;c>0;c--,s++,d++)
    *d |= *s & red_bitmask;

  unsigned char v = (unsigned int) *(float*)(params->begin()->value); // only one value

  SDL_imageFilterBinarizeUsingThreshold
    ((unsigned char*)dst,(unsigned char*)dst,bytes, v);
}

BLIT green_mask(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  register int c;
  register uint32_t *s = (uint32_t*)src;
  register uint32_t *d = (uint32_t*)dst;

  for(c=bytes>>2;c>0;c--,s++,d++)
    *d |= *s & green_bitmask;

  unsigned char v = (unsigned int) *(float*)(params->begin()->value); // only one value

  SDL_imageFilterBinarizeUsingThreshold
    ((unsigned char*)dst,(unsigned char*)dst,bytes, v);
}

BLIT blue_mask(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  register int c;
  register uint32_t *s = (uint32_t*)src;
  register uint32_t *d = (uint32_t*)dst;

  for(c=bytes>>2;c>0;c--,s++,d++)
    *d |= *s & blue_bitmask;

  unsigned char v = (unsigned int) *(float*)(params->begin()->value); // only one value
  
  SDL_imageFilterBinarizeUsingThreshold
    ((unsigned char*)dst,(unsigned char*)dst,bytes,v);
}


BLIT schiffler_add(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  SDL_imageFilterAdd((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_sub(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  SDL_imageFilterSub((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_mean(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  SDL_imageFilterMean((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_absdiff(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  SDL_imageFilterAbsDiff((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_mult(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  SDL_imageFilterMult((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_multnor(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  SDL_imageFilterMultNor((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_div(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  SDL_imageFilterDiv((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_multdiv2(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  SDL_imageFilterMultDivby2((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_multdiv4(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  SDL_imageFilterMultDivby2((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_and(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  SDL_imageFilterBitAnd((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

BLIT schiffler_or(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  SDL_imageFilterBitOr((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

/* ====== end of transparent blits
   all the following blits can be considered effects
   they completely overwrite the underlying image */

/// Linear non-transparent blits

BLIT schiffler_neg(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  SDL_imageFilterBitNegation((unsigned char*)src,(unsigned char*)dst,bytes);
}

BLIT schiffler_addbyte(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  unsigned char v = (unsigned int) *(float*)(params->begin()->value); // only one value

  SDL_imageFilterAddByte((unsigned char*)src,(unsigned char*)dst,bytes, v);
}

BLIT schiffler_addbytetohalf(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  unsigned char v = (unsigned int) *(float*)(params->begin()->value); // only one value

  SDL_imageFilterAddByteToHalf((unsigned char*)src,(unsigned char*)dst,bytes, v);
}

BLIT schiffler_subbyte(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  unsigned char v = (unsigned int) *(float*)(params->begin()->value); // only one value

  SDL_imageFilterSubByte((unsigned char*)src,(unsigned char*)dst,bytes, v);
}

BLIT schiffler_shl(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  unsigned char v = (unsigned int) *(float*)(params->begin()->value); // only one value

  SDL_imageFilterShiftLeft((unsigned char*)src,(unsigned char*)dst,bytes, v);
}

BLIT schiffler_shlb(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  unsigned char v = (unsigned int) *(float*)(params->begin()->value); // only one value

  SDL_imageFilterShiftLeftByte((unsigned char*)src,(unsigned char*)dst,bytes,v);
}

BLIT schiffler_shr(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  unsigned char v = (unsigned int) *(float*)(params->begin()->value); // only one value

  SDL_imageFilterShiftRight((unsigned char*)src,(unsigned char*)dst,bytes,v);
}

BLIT schiffler_mulbyte(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  unsigned char v = (unsigned int) *(float*)(params->begin()->value); // only one value

  SDL_imageFilterMultByByte((unsigned char*)src,(unsigned char*)dst,bytes,v);
}

BLIT schiffler_binarize(void *src, void *dst, int bytes, Linklist<Parameter> *params) {
  unsigned char v = (unsigned int) *(float*)(params->begin()->value); // only one value
  SDL_imageFilterBinarizeUsingThreshold
    ((unsigned char*)src,(unsigned char*)dst,bytes, v);
}




void setup_linear_blits(Blitter *blitter) {
  Blit *b;
  Parameter *p;

  b = new Blit(); b->set_name("RGB");
  sprintf(b->desc, "RGB blit (jmemcpy)");
  b->type = Blit::LINEAR;
  b->fun = rgb_jmemcpy; blitter->blitlist.prepend(b);

  blitter->default_blit = b; // default is RGB

  b = new Blit(); b->set_name("ADD");
  sprintf(b->desc,"bytewise addition");
  b->type = Blit::LINEAR;
  b->fun = schiffler_add; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("SUB");
  sprintf(b->desc,"bytewise subtraction");
  b->type = Blit::LINEAR;
  b->fun = schiffler_sub; blitter->blitlist.prepend(b);
  
  b = new Blit(); b->set_name("MEAN");
  sprintf(b->desc,"bytewise mean");
  b->type = Blit::LINEAR;
  b->fun = schiffler_add; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("ABSDIFF");
  sprintf(b->desc,"absolute difference");
  b->type = Blit::LINEAR;
  b->fun = schiffler_absdiff; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("MULT");
  sprintf(b->desc,"multiplication");
  b->type = Blit::LINEAR;
  b->fun = schiffler_mult; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("MULTNOR");
  sprintf(b->desc,"normalized multiplication");
  b->type = Blit::LINEAR;
  b->fun = schiffler_multnor; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("DIV");
  sprintf(b->desc,"division");
  b->type = Blit::LINEAR;
  b->fun = schiffler_div; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("MULTDIV2");
  sprintf(b->desc,"multiplication and division by 2");
  b->type = Blit::LINEAR;
  b->fun = schiffler_multdiv2; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("MULTDIV4");
  sprintf(b->desc,"multiplication and division by 4");
  b->type = Blit::LINEAR;
  b->fun = schiffler_multdiv4; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("AND");
  sprintf(b->desc,"bitwise and");
  b->type = Blit::LINEAR;
  b->fun = schiffler_and; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("OR");
  sprintf(b->desc,"bitwise or");
  b->type = Blit::LINEAR;
  b->fun = schiffler_or; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("XOR");
  sprintf(b->desc,"bitwise xor");
  b->type = Blit::LINEAR;
  b->fun = blit_xor; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("RED");
  sprintf(b->desc,"red channel only blit");
  b->type = Blit::LINEAR;
  b->fun = red_channel; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("GREEN");
  sprintf(b->desc,"green channel only blit");
  b->type = Blit::LINEAR;
  b->fun = green_channel; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("BLUE");
  sprintf(b->desc,"blue channel only blit");
  b->type = Blit::LINEAR;
  b->fun = blue_channel; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("REDMASK");
  sprintf(b->desc,"red channel threshold mask");
  b->type = Blit::LINEAR; b->value = 200; // default
  b->fun = red_mask; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("GREENMASK");
  sprintf(b->desc,"green channel threshold mask");
  b->type = Blit::LINEAR; b->value = 200; // default
  b->fun = green_mask; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("BLUEMASK");
  sprintf(b->desc,"blue channel threshold mask");
  b->type = Blit::LINEAR; b->value = 200; // default
  b->fun = blue_mask; blitter->blitlist.prepend(b);

  b = new Blit(); b->set_name("NEG");
  sprintf(b->desc,"bitwise negation");
  b->type = Blit::LINEAR;
  b->fun = schiffler_neg; blitter->blitlist.prepend(b);

  /////////

  b = new Blit(); b->set_name("ADDB");
  sprintf(b->desc,"add byte to bytes");
  b->type = Blit::LINEAR;
  b->fun = schiffler_addbyte; blitter->blitlist.prepend(b);

  p = new Parameter(Parameter::NUMBER);
  strcpy(p->name, "byte increment");
  strcpy(p->description, "amount to sum to the byte");
  p->multiplier = 255.0;
  b->parameters.append(p);

  /////////

  b = new Blit(); b->set_name("ADDBH");
  sprintf(b->desc,"add byte to half");
  b->type = Blit::LINEAR;
  b->fun = schiffler_addbytetohalf; blitter->blitlist.prepend(b);

  p = new Parameter(Parameter::NUMBER);
  strcpy(p->name, "half byte increment");
  strcpy(p->description, "amount to sum to the half byte");
  p->multiplier = 127.0;
  b->parameters.append(p);

  /////////

  b = new Blit(); b->set_name("SUBB");
  sprintf(b->desc,"subtract byte to bytes");
  b->type = Blit::LINEAR;
  b->fun = schiffler_subbyte; blitter->blitlist.prepend(b);

  p = new Parameter(Parameter::NUMBER);
  strcpy(p->name, "byte decrement");
  strcpy(p->description, "amount to substract to the pixel bytes");
  p->multiplier = 255.0;
  b->parameters.append(p);

  /////////

  b = new Blit(); b->set_name("SHL");
  sprintf(b->desc,"shift left bits");
  b->type = Blit::LINEAR;
  b->fun = schiffler_shl; blitter->blitlist.prepend(b);

  p = new Parameter(Parameter::NUMBER);
  strcpy(p->name, "byte decrement");
  strcpy(p->description, "amount to substract to the pixel bytes");
  p->multiplier = 255.0;
  b->parameters.append(p);

  /////////

  b = new Blit(); b->set_name("SHLB");
  sprintf(b->desc,"shift left byte");
  b->type = Blit::LINEAR;
  b->fun = schiffler_shlb; blitter->blitlist.prepend(b);

  p = new Parameter(Parameter::NUMBER);
  strcpy(p->name, "shift bits");
  strcpy(p->description, "amount of left bit shifts to apply on each pixel's byte");
  p->multiplier = 8.0;
  b->parameters.append(p);

  /////////

  b = new Blit(); b->set_name("SHR");
  sprintf(b->desc,"shift right bits");
  b->type = Blit::LINEAR;
  b->fun = schiffler_shr; blitter->blitlist.prepend(b);

  p = new Parameter(Parameter::NUMBER);
  strcpy(p->name, "shift bits");
  strcpy(p->description, "amount of right bit shifts to apply on each pixel's byte");
  p->multiplier = 8.0;
  b->parameters.append(p);

  /////////

  b = new Blit(); b->set_name("MULB");
  sprintf(b->desc,"multiply by byte");
  b->type = Blit::LINEAR;
  b->fun = schiffler_mulbyte; blitter->blitlist.prepend(b);

  p = new Parameter(Parameter::NUMBER);
  strcpy(p->name, "byte multiplier");
  strcpy(p->description, "amount to multiply on each pixel's byte");
  p->multiplier = 255.0;
  b->parameters.append(p);

  /////////

  b = new Blit(); b->set_name("BIN");
  sprintf(b->desc,"binarize using threshold");
  b->type = Blit::LINEAR;
  b->fun = schiffler_binarize; blitter->blitlist.prepend(b);

  p = new Parameter(Parameter::NUMBER);
  strcpy(p->name, "threshold");
  strcpy(p->description, "binary threshold value");
  p->multiplier = 255.0;
  b->parameters.append(p);


}
