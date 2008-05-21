/*  FreeJ
 *  (c) Copyright 2008 Denis Roio aka jaromil <jaromil@dyne.org>
 *                     Pablo Martin aka caedes
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
 * $Id: $
 *
 */
#include <config.h>
#ifdef WITH_OPENGL

#include <stdlib.h>
#include <string.h>


#include <gl_screen.h>
#include <layer.h>
#include <jutils.h>

#include <GL/freeglut.h>

static Layer *layer;

static void doblit() {
  func("glut doblit");
  if(layer) {
    glColor3f (0., 0., 0.);
    glRasterPos2i (0,0);
    layer->lock();
    glDrawPixels (layer->geo.w, layer->geo.h, GL_BGRA, GL_UNSIGNED_BYTE, layer->offset);
    layer->unlock();
  }
}

GlScreen::GlScreen()
  : ViewPort() {
  
  bpp = 32;
  x_translation = 0;
  y_translation = 0;
  x_rotation = 0;
  y_rotation = 0;
  rotation = 0;
  zoom = 1.0;
  glwin = 0;
  opengl = true;

  create_gl_context = true;

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

GlScreen::~GlScreen() {
  glutDestroyWindow(glwin);
}

bool GlScreen::init(int width, int height) {

  char **argv;
  int argc = 0;

	w = width;
	h = height;
	bpp = 32;
	size = w*h*(bpp>>3);
	pitch = w*(bpp>>3);


	if(create_gl_context) {
	  float m_perspect[] = {-1.f, /* left */
				1.f,  /* right */
				-1.f, /* bottom */
				1.f,  /* top */
				1.f,  /* front */
				20.f};/* back */
	  int w, h;

// 	  /* set the viewport to the size of the sub frame */
// 	  glViewport(0, 0, w, h);
	  
// 	  /* set orthogonal projection, with a relative frame size of (2asp x 2) */
// 	  glMatrixMode(GL_PROJECTION);
// 	  glLoadIdentity();
// 	  glFrustum(m_perspect[0], m_perspect[1],       // left, right
// 		    m_perspect[2], m_perspect[3],                   // bottom, top
// 		    m_perspect[4], m_perspect[5]);                  // front, back
	  
// 	  /* reset texture matrix */
// 	  glMatrixMode(GL_TEXTURE);
// 	  glLoadIdentity();
	  
	  
// 	  /* set the center of view */
// 	  glMatrixMode(GL_MODELVIEW);
// 	  glLoadIdentity();
// 	  gluLookAt(0, 0, 4, 0, 0, 0, 0, 1, 0);
// 	  //glTranslatef(asp, 1, 0);
	  
	  
// 	  glEnable(GL_DEPTH_TEST);
// 	  glEnable(GL_AUTO_NORMAL);
// 	  glEnable(GL_NORMALIZE);
// 	  glShadeModel(GL_SMOOTH);
// 	  //glShadeModel(GL_FLAT);
	  
	  
// 	  /* disable everything that is enabled in other modules
// 	     this resets the ogl state to its initial conditions */
// 	  glDisable(GL_LIGHTING);
// 	  for (int i=0; i<8; i++) glDisable(GL_LIGHT0 + i);
// 	  glDisable(GL_COLOR_MATERIAL);
	  
	  
	  glutInit (&argc, argv);
	  glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE);
	  glutInitWindowSize ( width, height);
	  glwin = glutCreateWindow (PACKAGE);
	  glutDisplayFunc(doblit);
	  glutIdleFunc(doblit);
	  glutShowWindow();
	}
//         screen = SDL_CreateRGBSurface
//          (sdl_flags,w,h,bpp,blue_bitmask,green_bitmask,red_bitmask,alpha_bitmask);
	// TODO screen : from where?

	return(true);
}

void GlScreen::resize(int resize_w, int resize_h) {

  // nop

}

void *GlScreen::coords(int x, int y) {
  // TODO : check to return the correct pixel buffer to draw on
  error("GlScreen:coords was called");
  return 
    ( x + (w*y) +
      (uint32_t*)screen->pixels );
}

bool GlScreen::check_opengl_error()
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
	else
	  return (true);

	return(false);
}

GLuint GlScreen::texturize(Layer *layer) {
  GLuint textureID;
  // generate texture for the layer
  glGenTextures( 1, &textureID );
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  layer->textureID = textureID;
  return textureID;
}
bool GlScreen::glblit(Layer *lay) {

  // real blit is done in the doblit glut callback
  layer = lay;

  //  When you put glutPostRedisplay(); somewhere in your code this means
  // that the function defined using glutDisplayFunc() will be called at
  // the next opportunity. It marks the current window as needing a redraw.
  glutPostRedisplay();

  glutSwapBuffers();

  return(true);
}

bool GlScreen::glblitX(Layer *layer) {
  
  glBindTexture( GL_TEXTURE_2D, layer->textureID );
  printf("bla\n");
  layer->lock();

  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,
		layer->geo.w, layer->geo.h, 
		0, GL_RGBA, GL_UNSIGNED_BYTE, layer->offset );

  layer->unlock();

  if(!check_opengl_error()) return(false);

  // reset opengl environment.
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glGetError ();
  glMatrixMode( GL_MODELVIEW );
  glGetError ();
  glLoadIdentity();
  glGetError ();
  glTranslatef( x_translation, y_translation, -1 );
  glGetError ();

  // change scale
  //  glScaled(zoom, zoom, zoom);
  //  zoom-=0.01;

  // draw the screen quad
  glInterleavedArrays( GL_T2F_V3F, 0, g_quadVertices );
  glGetError ();
  glDrawArrays( GL_QUADS, 0, 4 );
  if(check_opengl_error())
    return(false);

  return(true);

}

void GlScreen::show() {
  func("GlScreen::show");
  if(create_gl_context)
    glutMainLoopEvent();
  // nop
}

void *GlScreen::get_surface() {
  error("GlScreen:get_surface called");
  // TODO: which pixels?
  return screen->pixels;
}



void GlScreen::clear() {
  // nop
}
void GlScreen::fullscreen() {
  // nop
}

bool GlScreen::lock() {
  // TODO
  return(true);
}

bool GlScreen::unlock() {
  // TODO
  return true;
}

int GlScreen::setres(int wx, int hx) {
  // nop
  return 1;
}

void GlScreen::set_magnification(int algo) {
  // nop
}
    
#endif
