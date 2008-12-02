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
import random
import math

freejc = freej.Context()
freejc.init(512,512,2,0)
freejc.plugger.refresh(freejc)

freejc2 = freej.Context()
freejc2.init(512,512,2,0)
freejc2.plugger.refresh(freejc2)


for gen in range(1,freejc.generators.len()+1):
    print freejc.generators[gen].name

def create_layer(ctx,name):
    layer = freej.GenF0rLayer()
    layer.init(ctx)
    layer.open("partik0l")
    layer.start()
    ctx.add_layer(layer)
    time.sleep(0.1)

    for a in range(1,layer.generator.proto.parameters.len()+1):
        par = layer.generator.proto.parameters[a]
        val = 1.0*a*random.random()
        print " *",par.name,par.description,val
        par.setValue(val)
        layer.generator.set_parameter(a)
    return layer

layer = create_layer(freejc,"partik0l")
layer2 = create_layer(freejc2,"partik0l")

clearscreen = False
doecho = False
distort = False
rotate = True

xrot = yrot = zrot = 0.0
textures = [0,1,2]

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
    global textures
    # nehe texture
    texturefile = os.path.join('..','..','..','doc','ipernav.png')
    textureSurface = pygame.image.load(texturefile)
    textureData = pygame.image.tostring(textureSurface, "RGBX", 1)

    textures = glGenTextures(3)
    glBindTexture(GL_TEXTURE_2D, textures[0])
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, textureSurface.get_width(), textureSurface.get_height(), 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, textureData );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)

    # freej texture
    layers = [None,layer,layer2]
    for idx in [1,2]:
        glBindTexture(GL_TEXTURE_2D, textures[idx])
        lay = layers[idx]
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, lay.geo.w, lay.geo.h, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, None );
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)


def draw_freej_layer(ctx,layer,texturen):
    # draw freej
    #print "draw",ctx,layer,texturen
    glBindTexture(GL_TEXTURE_2D, textures[texturen])
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glLoadIdentity()
    glTranslatef(0.0,0.0,-1.0) # XXX if this is not used, doesnt draw :P
    glPixelZoom(-1.0,-1.0)
    glViewport(layer.geo.w/2,layer.geo.h/2,layer.geo.w,layer.geo.h)
    ctx.cafudda(0.0)
    glColor3f (1.0, 1.0, 1.0)
    glPixelZoom(1.0,1.0)
    glViewport(0,0,layer.geo.w,layer.geo.h)
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 0, 0, layer.geo.w,layer.geo.h, 0)
    glViewport(0,0,800,600)
    # end draw freej

def draw():
    global xrot, yrot, zrot
    # alpha blending
    glBlendFunc( GL_SRC_ALPHA, GL_ONE )

    if random.random() < 0.5:
        draw_freej_layer(freejc,layer,1)
        draw_freej_layer(freejc2,layer2,2)
    else:
        draw_freej_layer(freejc2,layer2,2)
        draw_freej_layer(freejc,layer,1)

    # add blending
    glBlendFunc( GL_ONE, GL_ONE )

    # clear screen (or not)
    if clearscreen:
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    else:
        glClear(GL_DEPTH_BUFFER_BIT)

    if doecho:
        sep = math.sin(time.time())
        for i in range(10):
            draw_cube(sep*5*i,1.0/(i+1))
    else:
        draw_cube(0.0,1.0)

    if rotate:
        xrot = xrot + 0.1
        yrot = yrot + 0.1
        zrot = zrot + 0.1

def draw_cube(ang,col):
    glColor3f (col,col,col)
    # now typical stuff
    glLoadIdentity()
    glTranslatef(0.0,0.0,-5.0)

    glRotatef(xrot+ang,1.0,0.0,0.0)
    glRotatef(yrot+ang,0.0,1.0,0.0)
    glRotatef(zrot+ang,0.0,0.0,1.0)

    #scale = (1/(col-0.51))*0.1
    #glScalef(scale,scale,scale)
    
    glDisable(GL_DEPTH_TEST)

    glBindTexture(GL_TEXTURE_2D, textures[0])
    glBegin(GL_QUADS)
    # Front Face (note that the texture's corners have to match the quad's corners)
    glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0,  1.0)	# Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0, 0.0); glVertex3f( 1.0, -1.0,  1.0)	# Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0, 1.0); glVertex3f( 1.0,  1.0,  1.0)	# Top Right Of The Texture and Quad
    glTexCoord2f(0.0, 1.0); glVertex3f(-1.0,  1.0,  1.0)	# Top Left Of The Texture and Quad
    glEnd();				
	
    glBindTexture(GL_TEXTURE_2D, textures[1])
    glBegin(GL_QUADS)
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
    glBindTexture(GL_TEXTURE_2D, textures[2])
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
    
        

def main():
    global clearscreen,doecho,rotate
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
            if event.key == K_2:
                doecho = 1-doecho
            if event.key == K_3:
                rotate = 1-rotate
        draw()
        pygame.display.flip()
        frames = frames+1

    print "fps:  %d" % ((frames*1000)/(pygame.time.get_ticks()-ticks))


if __name__ == '__main__': main()
