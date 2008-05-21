import freej
import time

freej.set_debug(3)

# init context
cx = freej.Context()
cx.init(400,300,0,0)
cx.config_check("keyboard.js")
cx.plugger.refresh(cx)

# add a layer
lay = freej.create_layer(cx,"/home/caedes/dvgrab-001.avi")
lay.start()
lay.add_filter(cx.filters["vertigo"])
lay.active = True
cx.add_layer(lay)

# main loop
while True:
	cx.cafudda(1.0)
	time.sleep(0.04)
time.sleep(10)

