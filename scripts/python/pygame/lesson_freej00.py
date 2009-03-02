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
#freej.set_debug(3)
freejc = freej.Context()
freejc.init(512,512,2,0)
freejc.plugger.refresh(freejc)
layer = freejc.open(sys.argv[1])
#layer.fps.set(25)
filt = freejc.filters["vertigo"]
filt.apply(layer)
layer.start()
layer.active = True
freejc.add_layer(layer)
time.sleep(0.1)

# end init freej

xrot = yrot = zrot = 0.0
textures = [0,0]

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

def load_textures():
    glBindTexture(GL_TEXTURE_2D, textures[0])
    # create texture with freej layer size
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, layer.geo.w, layer.geo.h, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, None );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)


def draw():
    global xrot, yrot, zrot

    # draw freej
    glClearColor(1,0,0,1)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glLoadIdentity()
    glColor3f (1.0, 1.0, 1.0)
    freejc.cafudda(0.0)
    glColor3f (1.0, 1.0, 1.0)

def main():

    video_flags = OPENGL|DOUBLEBUF
    
    pygame.init()
    surface = pygame.display.set_mode((640,480), video_flags)

    resize((640,480))
    init()

    frames = 0
    ticks = pygame.time.get_ticks()
    while 1:
        event = pygame.event.poll()
        if event.type == QUIT or (event.type == KEYDOWN and event.key == K_ESCAPE):
            break
        
        draw()
        pygame.display.flip()
        frames = frames+1

    print "fps:  %d" % ((frames*1000)/(pygame.time.get_ticks()-ticks))


if __name__ == '__main__': main()
