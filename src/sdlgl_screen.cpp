/*  FreeJ
 *  (c) Copyright 2001 Denis Roio aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * "$Id$"
 *
 */
#include <config.h>

#ifdef WITH_OPENGL

#include <stdlib.h>
#include <string.h>
#include <SDL_syswm.h>
#include <SDL_opengl.h>

#include <sdlgl_screen.h>
#include <jutils.h>





SdlGlScreen::SdlGlScreen()
  : ViewPort() {
  
  emuscr = NULL;
  bpp = 32;
  dbl = false;
  sdl_flags = (SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_OPENGLBLIT | SDL_RESIZABLE | SDL_HWPALETTE | SDL_HWSURFACE );
  x_translation = 0;
  y_translation = 0;
  x_rotation = 0;
  y_rotation = 0;
  rotation = 0;
  zoom = 1.0;

  g_quadVertices[0].tu = 0.0f;
  g_quadVertices[0].tv = 1.0f;
  g_quadVertices[0].x = -1.0f;
  g_quadVertices[0].y = -1.0f;
  g_quadVertices[0].z = 0.0f;

  g_quadVertices[1].tu = 1.0f;
  g_quadVertices[1].tv = 1.0f;
  g_quadVertices[1].x = 1.0f;
  g_quadVertices[1].y = -1.0f;
  g_quadVertices[1].z = 0.0f;

  g_quadVertices[2].tu = 1.0f;
  g_quadVertices[2].tv = 0.0f;
  g_quadVertices[2].x = 1.0f;
  g_quadVertices[2].y = 1.0f;
  g_quadVertices[2].z = 0.0f;

  g_quadVertices[3].tu = 0.0f;
  g_quadVertices[3].tv = 0.0f;
  g_quadVertices[3].x = -1.0f;
  g_quadVertices[3].y = 1.0f;
  g_quadVertices[3].z = 0.0f;


  // add above | SDL_FULLSCREEN to go fullscreen from the start
    
    magnification = 0;
}

SdlGlScreen::~SdlGlScreen() {
  SDL_Quit();
}

bool SdlGlScreen::init(int width, int height) {
	char temp[120];

	/* initialize SDL */

	setenv("SDL_VIDEO_HWACCEL", "1", 1);  

	if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_EVENTTHREAD) < 0 ) {
		error("Can't initialize SDL: %s",SDL_GetError());
		return(false);
	}

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); // Enable OpenGL Doublebuffering
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);		//Use at least 5 bits of Red
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);	//Use at least 5 bits of Green
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);		//Use at least 5 bits of Blue
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);	//Use at least 16 bits for the depth buffer

	setres(width,height);

	// init open GL
	{
		glLoadIdentity();
		glDisable(GL_BLEND); 
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);
		glEnable( GL_TEXTURE_2D );
		
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glClearColor( 0.0f,0.0f,0.0f,0.0f );

		/* antialiasing
		glShadeModel (GL_SMOOTH);
		glEnable (GL_POLYGON_SMOOTH);
		glEnable (GL_LINE_SMOOTH);
		glEnable (GL_POINT_SMOOTH);
*/
		glClearDepth(1.0f);
		glDepthFunc(GL_LEQUAL);	
		glEnable(GL_DEPTH_TEST);
		glShadeModel(GL_SMOOTH);
		glDisable(GL_CULL_FACE);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
//		gluPerspective( 45.0f, (float)width / height, 0.1f, 10000.0f);
	}
	// generate texture
	glGenTextures( 1, &textureID );
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	surface = SDL_GetVideoSurface();

	w = width;
	h = height;
	bpp = 32;
	size = w*h*(bpp>>3);
	pitch = w*(bpp>>3);
	SDL_VideoDriverName(temp,120);

	notice("SDLGL Viewport is %s %ix%i %ibpp",
			temp,w,h,surface->format->BytesPerPixel<<3);

        screen = SDL_CreateRGBSurface
         (sdl_flags,w,h,bpp,blue_bitmask,green_bitmask,red_bitmask,alpha_bitmask);
	/* be nice with the window manager */
	sprintf(temp,"%s %s",PACKAGE,VERSION);
	SDL_WM_SetCaption (temp, temp);

	/* hide mouse cursor */
	SDL_ShowCursor(SDL_DISABLE);
	return(true);
}

void SdlGlScreen::resize(int resize_w, int resize_h) {
  surface = SDL_SetVideoMode(resize_w,resize_h,32,sdl_flags);
  w = resize_w;
  h = resize_h;
  size = resize_w * resize_h * (bpp>>3);
  pitch = resize_w * (bpp>>3);
}

void *SdlGlScreen::coords(int x, int y) {
  return 
    ( x + (w*y) +
      (uint32_t*)screen->pixels );
}

void SdlGlScreen::check_opengl_error()
{
	GLenum err = glGetError ();
	if(err == GL_INVALID_ENUM)
		notice("GL_INVALID_ENUM");
	else if (err == GL_INVALID_VALUE)
		notice("GL_INVALID_VALUE di glTexImage2D");
	else if (err == GL_INVALID_OPERATION)
		notice("GL_INVALID_OPERATION");
	else if (err == GL_STACK_OVERFLOW)
		notice("GL_STACK_OVERFLOW");
	else if (err == GL_STACK_UNDERFLOW)
		notice("GL_STACK_UNDERFLOW");
	else if (err == GL_OUT_OF_MEMORY)
		notice("GL_OUT_OF_MEMORY");
	else if (err == GL_TABLE_TOO_LARGE)
		notice("GL_TABLE_TOO_LARGE");

}

void SdlGlScreen::blit(Layer *lay) {
	// bind freej texture and copy it
	glBindTexture( GL_TEXTURE_2D, textureID );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, screen->w, screen->h, 
			  0, GL_RGBA, GL_UNSIGNED_BYTE, screen->pixels );
	check_opengl_error();

	// reset opengl environment.
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glGetError ();
	glMatrixMode( GL_MODELVIEW );
	glGetError ();
	glLoadIdentity();
	glGetError ();
	glTranslatef( x_translation, y_translation, -1 );
	glGetError ();
	
	// rotation
	/*
	glRotatef( y_rotation, 0.0f, 1.0f, 0.0f );
	glRotatef( x_rotation, 1.0f, 0.0f, 0.0f );

	glRotatef( rotation, 0.0f, 0.0f, 1.0f );
	*/
	// change scale
	//glScaled(zoom, zoom, zoom);
	//zoom-=0.01;

	// draw the screen quad
	glInterleavedArrays( GL_T2F_V3F, 0, g_quadVertices );
	glGetError ();
	glDrawArrays( GL_QUADS, 0, 4 );
	check_opengl_error();
}

void SdlGlScreen::show() {
  //	drawframe();
	SDL_GL_SwapBuffers();
	check_opengl_error();
}

void *SdlGlScreen::get_surface() {
  return screen->pixels;
}



void SdlGlScreen::clear() {
  SDL_FillRect(screen,NULL,0x0);
}
void SdlGlScreen::fullscreen() {
  SDL_WM_ToggleFullScreen(surface);
}

void SdlGlScreen::lock() {
  if (!SDL_MUSTLOCK(screen)) 
    if (SDL_LockSurface(screen) < 0) 
      error("%s", SDL_GetError());
}

void SdlGlScreen::unlock() {
  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);
}

int SdlGlScreen::setres(int wx, int hx) {
  /* check and set available videomode */
  int res;
  res = SDL_VideoModeOK(wx, hx, bpp, sdl_flags);
  
  
  surface = SDL_SetVideoMode(wx, hx, bpp, sdl_flags);
  //  screen = SDL_SetVideoMode(wx, hx, 0, sdl_flags);
  if( surface == NULL ) {
    error("can't set video mode: %s\n", SDL_GetError());
    return(false);
  }


  if(res!=bpp) {
    act("your screen does'nt support %ubpp",bpp);
    act("doing video surface software conversion");
    
    emuscr = SDL_GetVideoSurface();
    act("emulated surface geometry %ux%u %ubpp",
	emuscr->w,emuscr->h,emuscr->format->BitsPerPixel);
  } 
 

  return res;
}

void SdlGlScreen::set_magnification(int algo) {

  if(magnification == algo) return;


  if(algo==0) {
    notice("screen magnification off");
    setres(w,h);
    if(magnification) SDL_FreeSurface(surface);
    surface = SDL_GetVideoSurface();

  } else if(algo==1) {

    notice("screen magnification scale2x");
    setres(w*2,h*2);

  } else if(algo==2) {

    notice("screen magnification scale3x");
    setres(w*3,h*3);

  } else {

    error("magnification algorithm %i not supported",algo);
    algo = magnification;

  }


  if(!magnification && algo) {
    func("create surface for magnification");
    screen = SDL_CreateRGBSurface
      (sdl_flags,w,h,bpp,blue_bitmask,green_bitmask,red_bitmask,alpha_bitmask);
      //      (SDL_HWSURFACE,w,h,bpp,blue_bitmask,green_bitmask,red_bitmask,alpha_bitmask);
  }

  magnification = algo;
  
}
    
#endif
