import freej
import gtk
import threading
import gtk.glade
gtk.gdk.threads_init ()

class MyConsole(freej.ConsoleController):
    def __init__(self, textview):
        self.textview = textview
        freej.ConsoleController.__init__(self)
        print "init"
    def poll(self):
        return 0
    def dispatch(self):
        return 0
    def notice(self, msg):
        iter = self.textview.get_buffer().get_end_iter()
        self.textview.get_buffer().insert(iter, msg+'\n')
    def error(self, msg):
        iter = self.textview.get_buffer().get_end_iter()
        self.textview.get_buffer().insert(iter, msg+'\n')
    def warning(self, msg):
        iter = self.textview.get_buffer().get_end_iter()
        self.textview.get_buffer().insert(iter, msg+'\n')
    def act(self, msg):
        print " X*",msg
    def func(self, msg):
        print " X*",msg
    def refresh(self, msg):
        print " X*",msg


# -----------------------------------------------------

class MyCall(freej.DumbCall):
   def __init__(self, func, *args):
       super(MyCall, self).__init__()
       self.func = func
       self.args = args

   def callback(self):
       self.func(*self.args)

class FreeJ(object):
    def __init__(self):
        self.cx = freej.Context()
        
        self.scr = freej.SdlScreen( 400, 300)
        
        self.cx.add_screen( self.scr )
        self.open_layer('/home/caedes/ca-midi.xavi')
        self.th = threading.Thread(target = self.cx.start , name = "freej")

        #cb = MyCall(self.finished)
        #v.add_eos_call(cb)

    def open_layer(self, filename):
        #v = freej.VideoLayer()
        print " * opening:",filename
        v = self.cx.open(filename)
        if not v:
            print " * cant open",filename
            return
        v.init(self.cx)
        v.open(filename)
        v.start()
        self.cx.add_layer( v )
        v.thisown = False

    def finished(self):
        print "video looping!"


class App(FreeJ):
    def __init__(self):
        self.fname = 'freej_mixer.glade'
        self.wTree = gtk.glade.XML(self.fname)
        self.window = self.wTree.get_widget("main")
        self.window.connect('destroy', gtk.main_quit)
        textview = self.wTree.get_widget("status")
        self.console = MyConsole(textview)
        freej.set_console(self.console)
        FreeJ.__init__(self)
        self.th.start();
        self.wTree.signal_autoconnect({"open_file":self.open_file})

    def open_file(self, *args):
        self.filew = gtk.FileChooserDialog("File selection",
                                          None,
                                          gtk.FILE_CHOOSER_ACTION_OPEN,
                                          (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                          gtk.STOCK_OPEN, gtk.RESPONSE_OK))
        self.filew.set_default_response(gtk.RESPONSE_OK)
        response = self.filew.run()
        if response == gtk.RESPONSE_OK:
            filename = self.filew.get_filename()
            self.open_layer(filename)
        self.filew.hide()

# -----------------------------------------------------


# -----------------------------------------------------

if __name__ == "__main__":
    app = App()
    gtk.main()
    print "exiting"
    app.cx.quit = False

