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
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>


#include <gl_screen.h>
#include <layer.h>
#include <jutils.h>

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(ViewPort, GlScreen, Screen, gl);

GlScreen::GlScreen()
  : ViewPort() {
  
  dbl = false;
  x_translation = 0;
  y_translation = 0;
  x_rotation = 0;
  y_rotation = 0;
  rotation = 0;
  zoom = 1.0;
  opengl = true;

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
 /* glGenTextures( 1, &textureID );
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  layer->textureID = textureID;*/
  return textureID;
}
void GlScreen::blit(Layer *layer) {
  //glColor3f (0., 0., 0.);
  //glRasterPos2i (0,0);
  layer->lock();
  glDrawPixels (layer->geo.w, layer->geo.h, GL_BGRA, GL_UNSIGNED_BYTE, layer->buffer);
  layer->unlock();
}

/*
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
*/


    
