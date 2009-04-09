import threading
import freej

class MyCall(freej.DumbCall):
  def __init__(self, *args):
    super(MyCall, self).__init__(*args)

  def callback(self):
    print "detected EOS from python"



W = 400
H = 300
cx = freej.Context()
cx.init(W,H,0,0)
cx.clear_all = True

th = threading.Thread(target = cx.start , name = "freej")
th.start();

v = freej.VideoLayer()
v.init(cx)
v.open('/home/jaromil/Movies/TheRevolutionWillNotBeTelevisedGilScottHeron.mp4')
v.fit()
v.start()
v.active = True

cx.add_layer(v)

cb = MyCall()
v.add_eos_call(cb)
