import freej
import os
import sys
import gtk
import gobject
import threading
import gtk.glade
import traceback
import gtksourceview2 as gtksourceview
gtk.gdk.threads_init ()
try:
    import numpy
    has_numpy = True
except:
    has_numpy = False
#freej.set_debug(3)

MENU = -1
CONTROLLER = 0
LAYER = 1
FILTER = 2

class MyConsole(freej.ConsoleController):
    def __init__(self, statuslist, statusbar):
        self.statuslist = statuslist
        #self.scrolledwindow = scroll
        self.infoicon = self.statuslist.render_icon(gtk.STOCK_INFO, gtk.ICON_SIZE_MENU)
        self.w_icon = self.statuslist.render_icon(gtk.STOCK_DIALOG_WARNING, gtk.ICON_SIZE_MENU)
        self.e_icon = self.statuslist.render_icon(gtk.STOCK_DIALOG_ERROR, gtk.ICON_SIZE_MENU)
        self.d_icon = self.statuslist.render_icon(gtk.STOCK_EXECUTE, gtk.ICON_SIZE_MENU)
        self.u_icon = self.statuslist.render_icon(gtk.STOCK_DIALOG_QUESTION, gtk.ICON_SIZE_MENU)
        self.statusmodel = statuslist.get_model()
        self.statusbar = statusbar
        self.status_id = self.statusbar.get_context_id('freej')
        self.statusbar.push(self.status_id, "FreeJ started")
        freej.ConsoleController.__init__(self)

    def poll(self):
        return 0

    def dispatch(self):
        return 0

    def notice(self, msg):
        """
        task opened
        """
        self.advance(self.infoicon, msg)

    def error(self, msg):
        """
        fatal error
        """
        self.advance(self.e_icon, msg)
    def warning(self, msg):
        """
        warning
        """
        self.advance(self.w_icon, msg)
    def act(self, msg):
        """
        normal message
        """
        self.advance(self.d_icon, msg)
    def func(self, msg):
        """
        debug
        """
        self.advance(self.u_icon, msg)

    def advance(self, icon, msg):
        iter = self.statusmodel.append([icon, msg])
        self.statusbar.push(self.status_id, msg)
        path = self.statuslist.get_model().get_path(iter)
        self.statuslist.scroll_to_cell(path, None)

    def refresh(self, msg):
        print " X*", msg


# -----------------------------------------------------

class MyCall(freej.DumbCall):
   def __init__(self, func, *args):
       super(MyCall, self).__init__()
       self.func = func
       self.args = args

   def callback(self):
       self.func(*self.args)

class ContextMenu(gtk.Menu):
    """
    A context menu for immediate use.
    Takes a list of button names, and a parent widget, and it
    will show itself, and set callbacks on the parent, on the form:
     on_cm_<butname>
    It will also replace spaces in the button name, and set lower letters.
    """
    def __init__(self, parent, buttons, icons=[]):
        gtk.Menu.__init__(self)
        for i, butname in enumerate(buttons):
            if icons and icons[i]:
                item = gtk.ImageMenuItem(icons[i])
                #item.set_text(butname)
            else:
                item = gtk.MenuItem(butname)
            socketname = "on_cm_" + butname.replace(" ", "_").lower()
            item.connect("activate", getattr(parent, socketname))
            item.show()
            self.append(item)

    def popup(self, selected_obj, time):
        self._selected = selected_obj
        gtk.Menu.popup(self, None, None, None, 3, time, selected_obj)



class FreeJ(object):
    def __init__(self):
        self.scr = freej.SdlScreen( 400, 300)
        self.init_context()

    def init_context(self):
        self.cx = freej.Context()
        self.cx.clear_all = True
        
        
        self.cx.add_screen( self.scr )
        self.layers = []
        if len(sys.argv) > 1:
            self.open_layer(sys.argv[1])
        self.th = threading.Thread(target = self.cx.start , name = "freej")
        self.th.start();

        #cb = MyCall(self.finished)
        #v.add_eos_call(cb)

    def delete_layer(self, layer, layer_idx=-1):
        self.scr.rem_layer(layer)
        try:
            self.layers.pop(layer_idx)
        except:
            pass
        try:
            freej.delete_layer(layer)
        except:
            pass # only on some branch


    def open_layer(self, filename):
        #v = freej.VideoLayer()
        print " * opening:",filename
        v = self.cx.open(filename)
        if not v:
            print " * cant open", filename
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

class JsBuffer(gtksourceview.Buffer):
    def __init__(self):
        gtksourceview.Buffer.__init__(self)
        lang_manager = gtksourceview.LanguageManager()
        lang = lang_manager.get_language('js')
        self.set_language(lang)
        self.filename = None

    def reset(self):
        self.filename = None
        self.set_property('text', "")

    def save(self, filename=None):
        if not filename:
            filename = self.filename
        else:
            self.filename = filename
        if filename:
            text = self.get_property('text')
            f = open(filename, 'w')
            f.write(text)
            f.close()
            print " * saved as", filename

    def load_file(self, filename):
        f = open(filename, 'r')
        self.set_property('text', f.read())
        f.close()
        self.filename = filename

class App(FreeJ):
    def __init__(self):
        self.fname = 'freej_mixer.glade'
        self.wTree = gtk.glade.XML(self.fname)
        self.window = self.wTree.get_widget("main")
        self.statusbar = self.wTree.get_widget("statusbar")
        #self.preview_scroll = self.wTree.get_widget("preview_scroll")
        #self.preview_scroll.hide()
        self.vbox2 = self.wTree.get_widget("vbox2")
        self.setup_editor()
        self.setup_file_dialogs()
        self.window.connect('destroy', gtk.main_quit)
        self.history_w = self.wTree.get_widget("window_history")
        self.history_w.connect('delete-event', self.hide_history)
        self.history_t = self.wTree.get_widget("textview_history")
        self.prepare_status()
        self.console = MyConsole(self.statustree, self.statusbar)
        freej.set_console(self.console)
        FreeJ.__init__(self)
        self.wTree.signal_autoconnect({"open_file": self.open_file,
                                       "open_script": self.open_script,
                                       "tree_button": self.on_treeview_button_press_event,
                                       "add_effect" : self.add_effect,
                                       "do_reset" : self.do_reset,
                                       "on_debug" : self.on_debug,
                                       "on_script_save" : self.on_script_save,
                                       "on_script_save_as" :
                                       self.on_script_save_as,
                                       "on_script_new" : self.on_script_new,
                                       "on_script_play" : self.on_script_play,
                                       "on_script_stop" : self.on_script_stop,
                                       "run_command": self.run_command,
                                       "show_preview": self.show_preview,
                                       "show_history": self.show_history})

        self.popup = ContextMenu(self, ["delete"])
        #data = self.cx.
        self.preview_box = None
        self.prepare_tree()
        self.fill_tree()
        self.fill_effects()

    def invert_array(self, data):
        if not has_numpy:
            return data
        datac = numpy.frombuffer(data, numpy.uint8)
        data = numpy.copy(datac)
        data[0::4], data[2::4] = datac[2::4], datac[0::4] 
        return data

    def update_previews(self):
        self.scr.lock()
        self.scr.layers.lock()
        data = self.scr.get_surface_buffer()
        w = self.scr.w
        h = self.scr.h
        #data = self.invert_array(data)
        data_array = []
        if len(self.scr.layers):
            # layer preview
            for layer in self.scr.layers:
                data = layer.get_surface_buffer()
                data = self.invert_array(data)
                w = layer.geo.w
                h = layer.geo.h
                data_array.append([data,w,h])
        else:
            return
        if self.preview_box:
            self.vbox2.remove(self.preview_box)
            self.preview_box = None
        self.preview_box = gtk.VBox()
        self.vbox2.pack_start(self.preview_box, expand=False)
        for data,w,h in data_array:
            self.pixbuf = gtk.gdk.pixbuf_new_from_data(data, gtk.gdk.COLORSPACE_RGB,
                                                   True, 8, w,
                                                   h, w*4)
            self.scr.layers.unlock()
            self.scr.unlock()
            self.pixbuf = self.pixbuf.scale_simple(200, 150, gtk.gdk.INTERP_BILINEAR)
            self.gtkpixmap = gtk.Image()
            self.gtkpixmap.set_from_pixbuf(self.pixbuf)
            self.preview_box.pack_start(self.gtkpixmap, expand=False)
            self.gtkpixmap.show()
        self.preview_box.show()
        #self.preview_scroll.add_with_viewport(self.preview_box)

    def __timeout(self, widget):
        self.update_previews()
        self._timeout_id = gobject.idle_add(self.__timeout, self.window)

    def on_script_play(self, button):
        self.on_script_save(button)
        if self.buffer.filename:
            self.cx.open_script(self.buffer.filename)
        self.fill_tree()

    def on_script_stop(self, button):
        self.do_reset(button)

    def on_script_save(self, button):
        if self.buffer.filename:
            self.buffer.save()
        else:
            self.on_script_save_as(button)

    def on_script_save_as(self, button):
        self.filew = gtk.FileChooserDialog("SaveAs",
                                          None,
                                          gtk.FILE_CHOOSER_ACTION_SAVE,
                                          (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                          gtk.STOCK_SAVE, gtk.RESPONSE_OK))
        self.filew.set_default_response(gtk.RESPONSE_OK)
        response = self.filew.run()
        if response == gtk.RESPONSE_OK:
            filename = self.filew.get_filename()
            self.buffer.save(filename)
        self.filew.hide()

    def on_script_new(self, button):
        self.buffer.reset()


    def setup_editor(self):
        self.content_pane = self.wTree.get_widget("text_scroll")
        self.buffer = JsBuffer()
        self.editor = gtksourceview.View(self.buffer)
        self.editor.set_show_line_numbers(True)

        self.content_pane.add(self.editor)
        self.editor.show()
        accel_group = gtk.AccelGroup()
        self.window.add_accel_group(accel_group)

        self.editor.add_accelerator("paste-clipboard",
                                               accel_group,
                                               ord('v'),
                                               gtk.gdk.CONTROL_MASK,
                                               0)
        self.editor.add_accelerator("copy-clipboard",
                                               accel_group,
                                               ord('c'),
                                               gtk.gdk.CONTROL_MASK,
                                               0)
        self.editor.add_accelerator("cut-clipboard",
                                               accel_group,
                                               ord('x'),
                                               gtk.gdk.CONTROL_MASK,
                                               0)

    def on_debug(self, menuitem):
        if menuitem.get_active():
            freej.set_debug(3)
        else:
            freej.set_debug(1)

    def on_cm_delete(self, args):
        model, iter = self.main_tree.get_selection().get_selected()
        obj_type = model.get_value(iter, 2)
        if obj_type == LAYER:
            layer_idx = model.get_value(iter, 1)
            layer = self.scr.layers[layer_idx+1]
            self.delete_layer(layer, layer_idx)
        elif obj_type == FILTER:
            self.delete_effect()
        elif obj_type == CONTROLLER:
            c_idx = model.get_value(iter, 1)
            ctrl = self.cx.controllers[c_idx+1]
            self.cx.rem_controller(ctrl)
        self.fill_tree()

    def on_treeview_button_press_event(self, treeview, event):
        if event.button == 3:
            x = int(event.x)
            y = int(event.y)
            time = event.time
            pthinfo = treeview.get_path_at_pos(x, y)
            if pthinfo is not None:
                path, col, cellx, celly = pthinfo
                treeview.grab_focus()
                treeview.set_cursor( path, col, 0)
                self.popup.popup( None, time)
            return 1

    def do_reset(self, button):
        #del self.cx
        #self.cx.reset()
        print " * delete controllers"
        for controller in list(self.cx.controllers):
            self.cx.rem_controller(controller)
        print " * delete layers"
        for layer in list(self.scr.layers):
            self.delete_layer(layer)
        print " * clear layers"
        self.layers = []
        print " * init context"
        #self.init_context()
        print " * fill tree"
        self.fill_tree()

    def add_effect(self, button):
        model, iter = self.main_tree.get_selection().get_selected()
        obj_type = model.get_value(iter, 2)
        if obj_type == LAYER:
            layer_idx = model.get_value(iter, 1)
            layer = self.scr.layers[layer_idx+1]
            idx = self.effects_cb.get_active()
            effect = self.cx.filters.pick(idx+1)
            self.eff = effect.apply(layer)
            self.fill_tree()

    def delete_effect(self, button=None):
        model, iter = self.main_tree.get_selection().get_selected()
        effect_name = model.get_value(iter, 0)
        parent_iter = model.iter_parent(iter)
        lay_idx = model.get_value(parent_iter, 1)
        layer = self.scr.layers[lay_idx+1]
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
        cell = gtk.CellRendererText()
        self.effects_cb.pack_start(cell, True)
        self.effects_cb.add_attribute(cell, 'text', 0)

    def prepare_tree(self):
        self.main_tree = self.wTree.get_widget("main_tree")
        self.main_model = gtk.TreeStore(str, int, int, gtk.gdk.Pixbuf)
        self.main_tree.set_model(self.main_model)
        self.folder_icon = self.main_tree.render_icon(gtk.STOCK_DIRECTORY, gtk.ICON_SIZE_MENU)
        self.layer_icon = self.main_tree.render_icon(gtk.STOCK_FILE, gtk.ICON_SIZE_MENU)
        self.effect_icon = self.main_tree.render_icon(gtk.STOCK_EXECUTE, gtk.ICON_SIZE_MENU)
        self.ctl_icon = self.main_tree.render_icon(gtk.STOCK_CONNECT, gtk.ICON_SIZE_MENU)
        cell = gtk.CellRendererText()
        px = gtk.CellRendererPixbuf()
        column = gtk.TreeViewColumn('name')
        column.pack_start(px, expand=False)
        column.pack_start(cell)
        self.main_tree.append_column(column)
        column.add_attribute(cell, "text", 0)
        column.add_attribute(px, "pixbuf", 3)
        print self.main_tree.get_model()

    def fill_tree(self):
        self.main_model.clear()
        controllers = self.main_model.append(None, ["Controllers", 0, MENU,
                                                    self.folder_icon])
        for c_idx, controller in enumerate(self.cx.controllers):
            c_iter = self.main_model.append(controllers, [controller.name,
                                                          c_idx, CONTROLLER,
                                                          self.ctl_icon])
        layers = self.main_model.append(None, ["Layers", 0, MENU,
                                               self.folder_icon])
        for l_idx, layer in enumerate(self.scr.layers):
            lay_iter = self.main_model.append(layers, [layer.name, l_idx, LAYER,
                                                      self.layer_icon])
            layer.set_blit("ADD")
            print layer.type
            for f_idx, filter in enumerate(layer.filters):
                iter = self.main_model.append(lay_iter, [filter.name, f_idx,
                                                         FILTER,
                                                         self.effect_icon])
        self.main_tree.expand_all()

    def hide_history(self, window, event):
        self.wTree.get_widget("history_menu").set_active(False)
        window.hide()
        return 1

    def show_history(self, checkitem):
        if checkitem.get_active():
            self.history_w.show()
        else:
            self.history_w.hide()

    def show_preview(self, checkitem):
        if checkitem.get_active():
            self._timeout_id = gobject.idle_add(self.__timeout, self.window)
        else:
            gobject.source_remove(self._timeout_id)
            if self.preview_box:
                self.vbox2.remove(self.preview_box)
                self.preview_box = None


    def run_command(self, entry):
        text = entry.get_text()
        self.cx.parse_js_cmd(text)
        entry.set_text("")
        iter = self.history_t.get_buffer().get_end_iter()
        self.history_t.get_buffer().insert(iter, text+"\n")

    def setup_file_dialogs(self):
        # video selection dialog
        self.filew = gtk.FileChooserDialog("Video selection",
                                          None,
                                          gtk.FILE_CHOOSER_ACTION_OPEN,
                                          (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                          gtk.STOCK_OPEN, gtk.RESPONSE_OK))
        self.filew.set_default_response(gtk.RESPONSE_OK)
        # script selection dialog
        self.files = gtk.FileChooserDialog("Script selection",
                                          None,
                                          gtk.FILE_CHOOSER_ACTION_OPEN,
                                          (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                          gtk.STOCK_OPEN, gtk.RESPONSE_OK))
        self.files.set_default_response(gtk.RESPONSE_OK)
        currdir = os.path.realpath(os.path.curdir)
        examplesdir = os.path.join(currdir, '..', '..', 'javascript', 'examples')
        if os.path.exists(examplesdir):
            self.files.set_current_folder(examplesdir)


    def open_file(self, *args):
        response = self.filew.run()
        if response == gtk.RESPONSE_OK:
            filename = self.filew.get_filename()
            self.open_layer(filename)
        self.filew.hide()
        self.fill_tree()

    def open_script(self, *args):
        response = self.files.run()
        if response == gtk.RESPONSE_OK:
            filename = self.files.get_filename()
            #self.cx.open_script(filename)
            self.buffer.load_file(filename)
        self.files.hide()
        self.fill_tree()

# -----------------------------------------------------


# -----------------------------------------------------

if __name__ == "__main__":
    app = App()
    gtk.main()
    print "exiting"
    app.cx.quit = False

