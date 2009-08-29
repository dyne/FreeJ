# to be run from the scripting gui

l = Screen.layers[1]
#print dir(l)
zoom = 1.3
l.set_zoom(zoom, zoom)
l.set_position(15,15)
l.set_blit('RGB')
#l.set_rotate(10)
#l.set_rotate(10)
#l.antialias = True
print l.zoom_x
print l.name
