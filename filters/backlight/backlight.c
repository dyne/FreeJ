/*
 * BackLight TV, ported from VisualJockey plugin.
 * GPL USE ONLY.  Original license follows.
 *
 */

/* Copyright (C) 2002 Pete Warden

Effect plugin for VisualJockey

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <freej.h>
#include <freej_plugin.h>

#include <SDL/SDL.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

/* DEFINE's by nullset@dookie.net */
#define RED(n)  ((n>>16) & 0x000000FF)
#define GREEN(n) ((n>>8) & 0x000000FF)
#define BLUE(n)  ((n>>0) & 0x000000FF)
#define RGB(r,g,b) ((0<<24) + (r<<16) + (g <<8) + (b))
#define INTENSITY(n)	( ( (RED(n)+GREEN(n)+BLUE(n))/3))

static char *name = "BackLight";
static char *author = "Pete Warden";
static char *info = "still don't know";
static int version = 1;

static ScreenGeometry *geo;

static void *procbuf;

float m_SpikeScale=127.0f;
static short aSin[512];
static int sin_index=0;
static int sin_index2=80;
static int movelight=1;
static int invert=0;

int init(ScreenGeometry *sg) {
  geo = sg;

  procbuf = malloc(geo->size);

  int i;
   
  for (i = 0; i < 512; i++) {
    float rad;
    int max;
    max = (geo->w>geo->h?geo->h:geo->w);
    rad =  (float)i * 0.0174532 * 0.703125; 
    aSin[i] = (short)(sin(rad)*max/2+max/2);
  } 
  return 1;
}

int clean() {
  return 1;
}

void *process(void *buffo) {

  const int nFixedShift=8;
  const int nFixedMult=(1<<nFixedShift);
  
  const int nWidth=geo->w;
  const int nHeight=geo->h;
  const int nHalfWidth=(movelight?aSin[sin_index]:(nWidth/2));
  const int nHalfHeight=(movelight?aSin[sin_index2]:(nHeight/2));
  
  const int nNumPixels = nWidth*nHeight;
  int xOrigin = 0;
  int yOrigin = 0;
  int nX=xOrigin;
  int nY=yOrigin;
  int32_t *pSource = (int32_t*)buffo;
  int32_t *pOutput = (int32_t*)procbuf;
  int32_t *pCurrentSource = pSource;
  int32_t *pCurrentOutput = pOutput;
  int32_t *pSourceEnd=pSource+nNumPixels;
  //ZeroMemory(pOutput,sizeof(int32_t)*nNumPixels);
  const int nSpikeScale= (int) (m_SpikeScale);
  bzero(pOutput, geo->size);
  
  while(pCurrentSource != pSourceEnd) {
    const int32_t* pSourceLineStart=pCurrentSource;
    const int32_t* pSourceLineEnd=pCurrentSource+nWidth;
    int nLength,nDeltaX,nDeltaY,nEndX,nEndY,nXInc,nYInc;
    int nCurrentX,nCurrentY,nDestXInc,nDestYInc;
    nX=xOrigin;
    while (pCurrentSource!=pSourceLineEnd) {
      int32_t SourceColour=*pCurrentSource;
      int32_t OutputColour=SourceColour;
      int nRed=RED(SourceColour);
      int nGreen= GREEN(SourceColour);
      int nBlue=BLUE(SourceColour);
      int32_t *pDest;
#ifdef FAST
      int nLuminance = INTENSITY(SourceColour);
#else
      int nLuminance = 90*nRed+115*nGreen+51*nBlue;
      nLuminance>>=8;
#endif
      if (invert) {
	nLuminance = 255-nLuminance;
      }
      SourceColour|=(nLuminance<<24);
      nLength=(nLuminance*nSpikeScale)>>nFixedShift;
      nDeltaX=((nX-nHalfWidth)*nLength)>>8;
      nDeltaY=((nY-nHalfHeight)*nLength)>>8;
      nEndX=nX+nDeltaX;
      nEndX=(nEndX>nWidth?nWidth:(nEndX<0?0:nEndX));
      nEndY=nY+nDeltaY;
      nEndY=(nEndX>nHeight?nHeight:(nEndY<0?0:nEndY));
      
      nXInc=(nDeltaX<0?-1:1);
      nYInc=(nDeltaY<0?-1:1);
      nDeltaX*=nXInc;
      nDeltaY*=nYInc;
      
      nCurrentX = nX;
      nCurrentY = nY;
      
      if ((nDeltaX==0)&&(nDeltaY==0)) {
	nDeltaX=1;
	nEndX+=1;
	nEndY+=1;
      } else if (nDeltaX==0) {
	nEndX+=1;
      } else if (nDeltaY==0) {
	nEndY+=1;
      }
      pDest=(pOutput+(nCurrentY*nWidth)+nCurrentX);
      nDestYInc=(nWidth*nYInc);
      nDestXInc=nXInc;
      if (nDeltaX>nDeltaY) {	      
	int nCounter=nDeltaY;
	while ((nCurrentX!=nEndX)&&(nCurrentY!=nEndY)) {
	  if ((pDest < pOutput+nNumPixels) &&
	      (pDest > pOutput)) {
	    const int32_t DestColour=*pDest;
	    if ((DestColour<SourceColour)) {
	      *pDest=SourceColour;
	    } else {
	      break;
	    }
	    if (nCounter>=nDeltaX) {
	      nCounter-=nDeltaX;
	      nCurrentY+=nYInc;
	      pDest+=nDestYInc;
	    }
	    nCurrentX+=nXInc;
	    pDest+=nDestXInc;
	    
	    nCounter+=nDeltaY;
	  } else
	    break;
	}
      } else {
	int	nCounter=nDeltaX;
	while ((nCurrentX!=nEndX)&&(nCurrentY!=nEndY)) {
	  if ((pDest < pOutput +nNumPixels) &&
	      (pDest >= pOutput)) {
	    const int32_t *DestColour=*pDest;
	    if ((DestColour<SourceColour)) {
	      *pDest=SourceColour;
	    } else {
	      break;
	    }
	    
	    if (nCounter>=nDeltaY) {
	      nCounter-=nDeltaY;
	      nCurrentX+=nXInc;
	      pDest+=nDestXInc;
	    }
	    nCurrentY+=nYInc;
	    pDest+=nDestYInc;
	    
	    nCounter+=nDeltaX;
	    
	  } else 
	    break;
	}
      }
      pCurrentSource+=1;
      
      pCurrentOutput++;
      nX+=1;
    }
    nY+=1;
  }
  sin_index+=3;
  sin_index &=511;
  sin_index2 +=5;
  sin_index2 &= 511;
  return procbuf;
}



int kbd_input(SDL_keysym *keysym) {
  int res = 1;

  switch(keysym->sym) {
  case SDLK_q:
    m_SpikeScale+=1.1;
    break;
  case SDLK_w:
    m_SpikeScale-=1.1;
    break;
  case SDLK_a:
    movelight=(movelight==0?1:0);
    break;
  case SDLK_s:
    invert=(invert==0?1:0);
  default:
    res = 0;
    break;
  }
  
  return res;
}
