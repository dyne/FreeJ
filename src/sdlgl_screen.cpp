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
 */

#include <config.h>

#ifdef WITH_OPENGL

#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL_opengl.h>

#include <sdlgl_screen.h>
#include <jutils.h>
#include "sdl_pixels.h"

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(ViewPort, SdlGlScreen, Screen, sdlgl);




SdlGlScreen::SdlGlScreen()
  : ViewPort() {
  
  window = nullptr;
  gl = nullptr;
  screen = nullptr;
  initialized_sdl = false;
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
    
  set_name("SDLGL");
}

SdlGlScreen::~SdlGlScreen() {
  if (gl)
    SDL_GL_DestroyContext(gl);
  SDL_DestroyWindow(window);
  SDL_DestroySurface(screen);
  if (initialized_sdl)
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
}

bool SdlGlScreen::_init() {
  if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
    error("Can't initialize SDL: %s", SDL_GetError());
    return false;
  }
  initialized_sdl = true;

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); // Enable OpenGL Doublebuffering
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);		//Use at least 5 bits of Red
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);	//Use at least 5 bits of Green
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);		//Use at least 5 bits of Blue
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);	//Use at least 16 bits for the depth buffer

	window = SDL_CreateWindow(PACKAGE " " VERSION, geo.w, geo.h, SDL_WINDOW_OPENGL);
  if (window)
    gl = SDL_GL_CreateContext(window);
  screen = SDL_CreateSurface(geo.w, geo.h, SDL_PIXELFORMAT_RGBA32);
  if (!window || !gl || !screen) {
    error("Can't create SDL OpenGL viewport: %s", SDL_GetError());
    return false;
  }

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

  notice("SDLGL Viewport is %s %ix%i %ibpp", SDL_GetCurrentVideoDriver(),
         geo.w, geo.h, geo.bpp);
  SDL_HideCursor();
	return(true);
}

void SdlGlScreen::setup_blits(Layer *lay) {
  Blitter *b = new Blitter();

  setup_linear_blits(b);

  lay->blitter = b;
}


void SdlGlScreen::resize(int resize_w, int resize_h) {
  auto *frame = SDL_CreateSurface(resize_w, resize_h, SDL_PIXELFORMAT_RGBA32);
  if (!frame || !SDL_SetWindowSize(window, resize_w, resize_h)) {
    error("Can't resize SDL OpenGL viewport: %s", SDL_GetError());
    SDL_DestroySurface(frame);
    return;
  }
  SDL_DestroySurface(screen);
  screen = frame;
  geo.init( resize_w, resize_h, 32);
}

void *SdlGlScreen::coords(int x, int y) {
  return 
    ( x + geo.pixelsize*y +
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
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, lay->geo.w, lay->geo.h, 
			  0, GL_RGBA, GL_UNSIGNED_BYTE, lay->buffer );
	check_opengl_error();

	// reset opengl environment.
// 	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
// 	glGetError ();
// 	glMatrixMode( GL_MODELVIEW );
// 	glGetError ();
// 	glLoadIdentity();
// 	glGetError ();
// 	glTranslatef( x_translation, y_translation, -1 );
// 	glGetError ();
	
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
// 	glInterleavedArrays( GL_T2F_V3F, 0, g_quadVertices );
// 	glGetError ();
// 	glDrawArrays( GL_QUADS, 0, 4 );
// 	check_opengl_error();
}

void SdlGlScreen::show() {
  //	drawframe();
	SDL_GL_SwapWindow(window);
	check_opengl_error();
}

void *SdlGlScreen::get_surface() {
  return screen->pixels;
}



void SdlGlScreen::clear() {
  SDL_FillSurfaceRect(screen,NULL,0x0);
}
void SdlGlScreen::fullscreen() {
  SDL_SetWindowFullscreen(window, !(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN));
}

bool SdlGlScreen::lock() {
  if (!SDL_MUSTLOCK(screen)) return true;
  if (!SDL_LockSurface(screen)) {
    error("%s", SDL_GetError());
    return false;
  }
  return(true);
}

bool SdlGlScreen::unlock() {
  if (SDL_MUSTLOCK(screen)) {
    SDL_UnlockSurface(screen);
  }
  return true;
}

#endif
