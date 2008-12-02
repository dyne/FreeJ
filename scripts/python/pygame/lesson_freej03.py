#!/usr/bin/env python
# pygame + PyOpenGL version of Nehe's OpenGL lesson06
# Paul Furber 2001 - m@verick.co.za

import os
import sys
from OpenGL.GL import *
from OpenGL.GLU import *
import pygame, pygame.image
from pygame.locals import *

# init freej and create a layer
import freej
import time
freejc = freej.Context()
freejc.init(512,512,2,0)
freejc.plugger.refresh(freejc)

for gen in range(1,freejc.generators.len()+1):
    print freejc.generators.pick(gen).name

layer = freej.GenF0rLayer()
layer.init(freejc)
layer.open("partik0l")
layer.start()
freejc.add_layer(layer)
time.sleep(0.1)

print layer.generator.proto.parameters
for a in range(1,layer.generator.proto.parameters.len()+1):
    par = layer.generator.proto.parameters[a]
    print " *",par.name,par.description
    par.setValue(0.2*a)
    layer.generator.set_parameter(a)

#filtn = freejc.filters.search("rotozoom")[1]
#filt = freejc.filters.getFilter(filtn)
#filt.apply(layer)
# end init freej

# keyboard globals
clearscreen = False

xrot = yrot = zrot = 0.0
textures = [0,1]

def resize((width, height)):
    if height==0:
        height=1
    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45, 1.0*width/height, 0.1, 100.0)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

def init():
    glEnable(GL_TEXTURE_2D)
    load_textures()
    glShadeModel(GL_SMOOTH)
    glClearColor(0.0, 0.0, 0.0, 0.0)
    glClearDepth(1.0)
    glEnable(GL_DEPTH_TEST)
    glDepthFunc(GL_LEQUAL)
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)
    glEnable(GL_BLEND)

def load_textures():

    # here set image file
    texturefile = os.path.join('..','..','..','doc','ipernav.png')
    textureSurface = pygame.image.load(texturefile)
    textureData = pygame.image.tostring(textureSurface, "RGBX", 1)

    glBindTexture(GL_TEXTURE_2D, textures[0])
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, textureSurface.get_width(), textureSurface.get_height(), 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, textureData );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)

    # freej texture
    glBindTexture(GL_TEXTURE_2D, textures[1])
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, layer.geo.w, layer.geo.h, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, None );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)

def draw():
    global xrot, yrot, zrot

    # alpha blending
    glBlendFunc( GL_SRC_ALPHA, GL_ONE )

    # draw freej
    glBindTexture(GL_TEXTURE_2D, textures[1])
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glLoadIdentity()
    glTranslatef(0.0,0.0,-1.0) # XXX if this is not used, doesnt draw :P
    glPixelZoom(-1.0,-1.0)
    glViewport(layer.geo.w/2,layer.geo.h/2,layer.geo.w,layer.geo.h)
    freejc.cafudda(0.0)
    glColor3f (1.0, 1.0, 1.0)
    glPixelZoom(1.0,1.0)
    glViewport(0,0,layer.geo.w,layer.geo.h)
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 0, 0, layer.geo.w,layer.geo.h, 0)
    glViewport(0,0,800,600)
    # end draw freej

    # add blending
    glBlendFunc( GL_ONE, GL_ONE )

    # clear screen (or not)
    if clearscreen:
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    else:
        glClear(GL_DEPTH_BUFFER_BIT)

    # now typical stuff
    glLoadIdentity()
    glTranslatef(0.0,0.0,-5.0)

    glRotatef(xrot,1.0,0.0,0.0)
    glRotatef(yrot,0.0,1.0,0.0)
    glRotatef(zrot,0.0,0.0,1.0)
    
    glDisable(GL_DEPTH_TEST)
    glBegin(GL_QUADS)
	
    # Front Face (note that the texture's corners have to match the quad's corners)
    glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0,  1.0)	# Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0, 0.0); glVertex3f( 1.0, -1.0,  1.0)	# Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0, 1.0); glVertex3f( 1.0,  1.0,  1.0)	# Top Right Of The Texture and Quad
    glTexCoord2f(0.0, 1.0); glVertex3f(-1.0,  1.0,  1.0)	# Top Left Of The Texture and Quad
	
    # Back Face
    glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, -1.0, -1.0)	# Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0, 1.0); glVertex3f(-1.0,  1.0, -1.0)	# Top Right Of The Texture and Quad
    glTexCoord2f(0.0, 1.0); glVertex3f( 1.0,  1.0, -1.0)	# Top Left Of The Texture and Quad
    glTexCoord2f(0.0, 0.0); glVertex3f( 1.0, -1.0, -1.0)	# Bottom Left Of The Texture and Quad
	
    # Top Face
    glTexCoord2f(0.0, 1.0); glVertex3f(-1.0,  1.0, -1.0)	# Top Left Of The Texture and Quad
    glTexCoord2f(0.0, 0.0); glVertex3f(-1.0,  1.0,  1.0)	# Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0, 0.0); glVertex3f( 1.0,  1.0,  1.0)	# Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0, 1.0); glVertex3f( 1.0,  1.0, -1.0)	# Top Right Of The Texture and Quad
    glEnd();				
	
    # Bottom Face       
    glBindTexture(GL_TEXTURE_2D, textures[0])
    glBegin(GL_QUADS)
    glTexCoord2f(1.0, 1.0); glVertex3f(-1.0, -1.0, -1.0)	# Top Right Of The Texture and Quad
    glTexCoord2f(0.0, 1.0); glVertex3f( 1.0, -1.0, -1.0)	# Top Left Of The Texture and Quad
    glTexCoord2f(0.0, 0.0); glVertex3f( 1.0, -1.0,  1.0)	# Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, -1.0,  1.0)	# Bottom Right Of The Texture and Quad
	
    # Right face
    glTexCoord2f(1.0, 0.0); glVertex3f( 1.0, -1.0, -1.0)	# Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0, 1.0); glVertex3f( 1.0,  1.0, -1.0)	# Top Right Of The Texture and Quad
    glTexCoord2f(0.0, 1.0); glVertex3f( 1.0,  1.0,  1.0)	# Top Left Of The Texture and Quad
    glTexCoord2f(0.0, 0.0); glVertex3f( 1.0, -1.0,  1.0)	# Bottom Left Of The Texture and Quad
	
    # Left Face
    glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0, -1.0)	# Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, -1.0,  1.0)	# Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0, 1.0); glVertex3f(-1.0,  1.0,  1.0)	# Top Right Of The Texture and Quad
    glTexCoord2f(0.0, 1.0); glVertex3f(-1.0,  1.0, -1.0)	# Top Left Of The Texture and Quad
	
    glEnd();				
    glEnable(GL_DEPTH_TEST)
    
    xrot = xrot + 0.02
    yrot = yrot + 0.02
    zrot = zrot + 0.02
        

def main():
    global clearscreen
    video_flags = OPENGL|DOUBLEBUF
    
    pygame.init()
    surface = pygame.display.set_mode((800,600), video_flags)

    resize((800,600))
    init()

    frames = 0
    ticks = pygame.time.get_ticks()
    while 1:
        event = pygame.event.poll()
        if event.type == QUIT or (event.type == KEYDOWN and event.key == K_ESCAPE):
            break
        elif event.type == KEYDOWN:
            if event.key == K_1:
                clearscreen = 1-clearscreen
        draw()
        pygame.display.flip()
        frames = frames+1

    print "fps:  %d" % ((frames*1000)/(pygame.time.get_ticks()-ticks))


if __name__ == '__main__': main()
