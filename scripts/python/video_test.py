import threading
import freej
import time
import sys

#freej.set_debug(3)

# init context
cx = freej.Context()
cx.init()
# init screen
scr = freej.SdlScreen()
scr.init( 400, 300 )

cx.add_screen(scr)

cx.config_check("keyboard.js")
cx.plugger.refresh(cx)

# add a layer
# lay = freej.create_layer(cx,sys.argv[1])
#lay = cx.open(sys.argv[1])
#filt = cx.filters["vertigo"]
#lay.add_filter(filt)

#header=freej.TextLayer()
#cx.add_layer(header)
#header.set_position(5,0)
#header.print_text("EngageMedia TV")
#header.start()

lay = freej.VideoLayer()
lay.init(cx)
lay.open("http://www.engagemedia.org/Members/swst/videos/triabunna.wmv")
lay.start()
lay.active = True
cx.add_layer(lay)
lay.set_position(10,10)


th = threading.Thread(target = cx.start , name = "freej")
th.start();
# th.join();

