import freej
import sys
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
        iter = self.textview.get_buffer().get_end_iter()
        self.textview.get_buffer().insert(iter, msg+'\n')
    def func(self, msg):
        iter = self.textview.get_buffer().get_end_iter()
        self.textview.get_buffer().insert(iter, msg+'\n')
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
        self.open_layer('/home/caedes/ca-midi.avi')
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
        self.history_w = self.wTree.get_widget("window_history")
        self.history_w.connect('delete-event', self.hide_history)
        self.history_t = self.wTree.get_widget("textview_history")
        textview = self.wTree.get_widget("status")
        self.console = MyConsole(textview)
        freej.set_console(self.console)
        FreeJ.__init__(self)
        self.th.start();
        self.wTree.signal_autoconnect({"open_file": self.open_file,
                                       "open_script": self.open_script,
                                       "run_command": self.run_command,
                                       "show_history": self.show_history})
        self.prepare_tree()
        self.fill_tree()

    def prepare_tree(self):
        self.main_tree = self.wTree.get_widget("main_tree")
        self.main_model = gtk.TreeStore(str)
        self.main_tree.set_model(self.main_model)
        cell = gtk.CellRendererText()
        column = gtk.TreeViewColumn('name', cell)
        column.add_attribute(cell, "text", 0)
        self.main_tree.append_column(column)
        print self.main_tree.get_model()

    def fill_tree(self):
        self.main_model.clear()
        for layer in self.scr.layers:
            print " * layer:",layer, layer.name
            lay_iter = self.main_model.append(None, [layer.name])
            for filter in layer.filters:
                print "  * filter" , filter, filter.name
                #iter = self.main_model.append(None, ["bla"])
                #self.main_model.append(None, ["foo"])
                #self.main_model.append(iter, ["bla2"])

    def hide_history(self, window, event):
        self.wTree.get_widget("history_menu").set_active(False)
        window.hide()
        return 1

    def show_history(self, checkitem):
        if checkitem.get_active():
            self.history_w.show()
        else:
            self.history_w.hide()

    def run_command(self, entry):
        text = entry.get_text()
        self.cx.parse_js_cmd(text)
        entry.set_text("")
        iter = self.history_t.get_buffer().get_end_iter()
        self.history_t.get_buffer().insert(iter, text+"\n")

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
        self.fill_tree()

    def open_script(self, *args):
        self.filew = gtk.FileChooserDialog("File selection",
                                          None,
                                          gtk.FILE_CHOOSER_ACTION_OPEN,
                                          (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                          gtk.STOCK_OPEN, gtk.RESPONSE_OK))
        self.filew.set_default_response(gtk.RESPONSE_OK)
        response = self.filew.run()
        if response == gtk.RESPONSE_OK:
            filename = self.filew.get_filename()
            self.cx.open_script(filename)
        self.filew.hide()
        self.fill_tree()

# -----------------------------------------------------


# -----------------------------------------------------

if __name__ == "__main__":
    app = App()
    gtk.main()
    print "exiting"
    app.cx.quit = False

