import freej
import sys
import gtk
import threading
import gtk.glade
gtk.gdk.threads_init ()
#freej.set_debug(3)


class MyConsole(freej.ConsoleController):
    def __init__(self, textview, statuslist):
        self.textview = textview
        self.statuslist = statuslist
        self.infoicon = self.statuslist.render_icon(gtk.STOCK_INFO, gtk.ICON_SIZE_MENU)
        self.w_icon = self.statuslist.render_icon(gtk.STOCK_DIALOG_WARNING, gtk.ICON_SIZE_MENU)
        self.e_icon = self.statuslist.render_icon(gtk.STOCK_DIALOG_ERROR, gtk.ICON_SIZE_MENU)
        self.d_icon = self.statuslist.render_icon(gtk.STOCK_EXECUTE, gtk.ICON_SIZE_MENU)
        self.u_icon = self.statuslist.render_icon(gtk.STOCK_DIALOG_QUESTION, gtk.ICON_SIZE_MENU)
        self.statusmodel = statuslist.get_model()
        freej.ConsoleController.__init__(self)
    def poll(self):
        return 0
    def dispatch(self):
        return 0
    def notice(self, msg):
        """
        task opened
        """
        iter = self.textview.get_buffer().get_end_iter()
        self.textview.get_buffer().insert(iter, msg+'\n')
        self.statusmodel.append([self.infoicon, msg])
    def error(self, msg):
        """
        fatal error
        """
        iter = self.textview.get_buffer().get_end_iter()
        self.textview.get_buffer().insert(iter, msg+'\n')
        self.statusmodel.append([self.e_icon, msg])
    def warning(self, msg):
        """
        warning
        """
        iter = self.textview.get_buffer().get_end_iter()
        self.textview.get_buffer().insert(iter, msg+'\n')
        self.statusmodel.append([self.w_icon, msg])
    def act(self, msg):
        """
        normal message
        """
        iter = self.textview.get_buffer().get_end_iter()
        self.textview.get_buffer().insert(iter, msg+'\n')
        self.statusmodel.append([self.d_icon, msg])
    def func(self, msg):
        """
        debug
        """
        iter = self.textview.get_buffer().get_end_iter()
        self.textview.get_buffer().insert(iter, msg+'\n')
        self.statusmodel.append([self.u_icon, msg])
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
        self.cx.clear_all = True
        
        self.scr = freej.SdlScreen( 400, 300)
        
        self.cx.add_screen( self.scr )
        self.layers = []
	if len(sys.argv) > 1:
	    self.open_layer(sys.argv[1])
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
        self.layers.append(v)
        return v

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
        self.prepare_status()
        textview = self.wTree.get_widget("status")
        self.console = MyConsole(textview, self.statustree)
        freej.set_console(self.console)
        FreeJ.__init__(self)
        self.th.start();
        self.wTree.signal_autoconnect({"open_file": self.open_file,
                                       "open_script": self.open_script,
                                       "add_effect" : self.add_effect,
                                       "del_effect" : self.del_effect,
                                       "run_command": self.run_command,
                                       "show_history": self.show_history})
        self.prepare_tree()
        self.fill_tree()
        self.fill_effects()

    def add_effect(self, button):
        idx = self.effects_cb.get_active()
        layer = self.layers[-1]
        effect = self.cx.filters.pick(idx+1)
        self.eff = effect.apply(layer)
        self.fill_tree()

    def del_effect(self, button):
        model, iter = self.main_tree.get_selection().get_selected()
        effect_name = model.get_value(iter, 0)
        layer = self.layers[-1]
        for idx, filter in enumerate(layer.filters):
            if filter.name == effect_name:
                layer.filters.rem(idx+1)
                filter.rem()
                self.fill_tree()
                return

    def prepare_status(self):
        self.statustree = self.wTree.get_widget("status_list")
        self.statusmodel = gtk.ListStore(gtk.gdk.Pixbuf, str)
        self.statustree.set_model(self.statusmodel)

        px = gtk.CellRendererPixbuf()
        text = gtk.CellRendererText()
        col = gtk.TreeViewColumn()
        col.pack_start(px, expand=False)
        col.pack_start(text, expand=True)
        col.add_attribute(px, "pixbuf", 0)
        col.add_attribute(text, "text", 1)
        self.statustree.append_column(col)

    def fill_effects(self):
        self.effects_cb = self.wTree.get_widget("combobox_effects")
        self.effect_model = gtk.ListStore(str)
        self.effects_cb.set_model(self.effect_model)
        self.cx.plugger.refresh(self.cx)
        for effect in self.cx.filters:
            self.effect_model.append([effect.name])
            #self.effect_model.append(["effect"])
            #self.effect_model.append(["effect1"])
            #self.effect_model.append(["effect2"])
            #self.effect_model.append(["effect3"])
        cell = gtk.CellRendererText()
        self.effects_cb.pack_start(cell, True)
        self.effects_cb.add_attribute(cell, 'text', 0)

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
            layer.set_blit("ADD")
            for filter in layer.filters:
                print "  * filter" , filter, filter.name
                iter = self.main_model.append(lay_iter, [filter.name])
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

