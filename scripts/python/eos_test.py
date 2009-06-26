import threading
import freej

class MyCall(freej.DumbCall):
  def __init__(self, *args):
    super(MyCall, self).__init__(*args)

  def callback(self):
    print "detected EOS of film (python callback)"

freej.set_debug(3)

W = 400
H = 300
cx = freej.Context()


scr = freej.SdlScreen( 400, 300)

cx.add_screen( scr )

v = freej.VideoLayer()

v.init(cx)

v.open('/home/jaromil/Movies/TheRevolutionWillNotBeTelevisedGilScottHeron.mp4')

v.start()

cx.add_layer( v )



cb = MyCall()
v.add_eos_call(cb)

th = threading.Thread(target = cx.start , name = "freej")
th.start();
