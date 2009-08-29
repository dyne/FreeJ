import threading
import freej
import time
import sys

#freej.set_debug(3)

# init context
cx = freej.Context()
cx.init()
#cx.init(400,300,0,0)
scr = freej.SdlScreen()
scr.init( 400, 300 );
cx.add_screen(scr)
cx.config_check("keyboard.js")
cx.plugger.refresh(cx)


# add a layer
# lay = freej.create_layer(cx,sys.argv[1])
lay = cx.open(sys.argv[1])
filt = cx.filters["vertigo"]
lay.add_filter(filt)
lay.start()
lay.active = True
cx.add_layer(lay)

th = threading.Thread(target = cx.start , name = "freej")
th.start();
# th.join();

