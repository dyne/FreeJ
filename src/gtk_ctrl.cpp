#include <SDL/SDL.h>
#include <context.h>
#include <jutils.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

#include <config.h>

/* widget -> callback connection define */
#define CONNECT(w,s,c) \
  wg = glade_xml_get_widget(gtk, w); \
  if(wg) g_signal_connect((gpointer) wg, s , \
			  G_CALLBACK( c ),NULL);

/* gtk_ctrl global variables */
static Context *env;
static GladeXML *gtk;
static GtkWidget *wg;
static GtkTreeIter iter;

/* layer's list model columns */
enum {
  LAYER_NAME,
  LAYER_FILE,
  LAYER_BLIT,
  LAYER_ACTIVE,
  LAYER_COLS
};


void on_fullscreen(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  SDL_WM_ToggleFullScreen(env->surf);
}
void on_about(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  env->osd->credits();
}
void on_select_layer(GtkWidget *widget, gpointer *data) {
  Layer *lay;
  char **sel = gtk_file_selection_get_selections(GTK_FILE_SELECTION(data));
  for(int c=0;sel[c];c++) {
    func("%s : creating %s",__FUNCTION__,sel[c]);
    lay = create_layer(sel[c]);
    if(lay) env->add_layer(lay);
  }
}
void on_add_layer(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  /* create a file selector dialog */
  GtkWidget *fs = gtk_file_selection_new("Add a new Layer from file:");
  /* register callback when ok is pressed */
  g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button),"clicked",
		   G_CALLBACK(on_select_layer), (gpointer)fs);
  /* destroy the fileselector when a button is pressed */
  g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button),"clicked",
			   G_CALLBACK(gtk_widget_destroy),(gpointer)fs);
  g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button),"clicked",
			   G_CALLBACK(gtk_widget_destroy),(gpointer)fs);
  gtk_file_selection_set_select_multiple(GTK_FILE_SELECTION(fs),true);
  /* show the widget */
  gtk_widget_show(fs);
}
void on_osd(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  //  bool res = gtk_toggle_button_get_active
  //    ((GtkToggleButton*)widget);
  env->osd->active();
}
void on_overwrite(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  env->clear_all = !env->clear_all;
}

bool gtk_ctrl_init(Context *nenv, int *argc, char **argv) {
  env = nenv;
  gtk_init(argc, &argv);
  gtk = glade_xml_new("../freej-gtk2.glade",NULL,NULL);
  
  /* connect signal handlers 
     glade_xml_signal_autoconnect(gtk); */

  /* signal to glib we're going to use threads */
  g_thread_init(NULL);

  /* connect signals to callbacks */
  CONNECT("fullscreen","activate",on_fullscreen);
  CONNECT("about","activate",on_about);
  CONNECT("add_layer","activate",on_add_layer);
  CONNECT("osd_switch","toggled",on_osd);
  CONNECT("overwrite","toggled",on_overwrite);

  /* =================== LAYER LIST */
  GtkWidget *layer_list;
  GtkListStore *layer_model;
  /* setup the layer's list model */
  layer_model =  gtk_list_store_new(LAYER_COLS,
				    G_TYPE_STRING, /* name */
				    G_TYPE_STRING, /* file */
				    G_TYPE_STRING, /* blit */
				    G_TYPE_BOOLEAN); /* on */
  layer_list = glade_xml_get_widget(gtk,"treeview_layer");
  /* register the model on the list view */
  gtk_tree_view_set_model((GtkTreeView*)layer_list,GTK_TREE_MODEL(layer_model));
  /* we can discard the reference to the model now */
  g_object_unref(G_OBJECT(layer_model));
  
  /* initialize tree view columns and renderers */
  GtkCellRenderer *rend;
  GtkTreeViewColumn *col;
  rend = gtk_cell_renderer_text_new();
  g_object_set(G_OBJECT(rend),"background","orange",NULL);
  col = gtk_tree_view_column_new_with_attributes
    ("Name",rend,"text",LAYER_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(layer_list),col);
  rend = gtk_cell_renderer_text_new();
  col = gtk_tree_view_column_new_with_attributes
    ("File",rend,"text",LAYER_FILE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(layer_list),col);  
  rend = gtk_cell_renderer_text_new();
  g_object_set(G_OBJECT(rend),"background","gray",NULL);
  col = gtk_tree_view_column_new_with_attributes
    ("Blit",rend,"text",LAYER_BLIT,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(layer_list),col);  
  rend = gtk_cell_renderer_toggle_new();
  col = gtk_tree_view_column_new_with_attributes
    ("ON",rend,"active",LAYER_ACTIVE);
  gtk_tree_view_append_column(GTK_TREE_VIEW(layer_list),col);

  /* fill up the list of layers allready loaded */
  Layer *lay = (Layer*)env->layers.begin();
  while(lay) {
    gtk_list_store_append(layer_model,&iter);
    gtk_list_store_set(layer_model,&iter,
		       LAYER_NAME,lay->getname(),
		       LAYER_FILE,lay->get_filename(),
		       LAYER_BLIT,lay->get_blit(),
		       LAYER_ACTIVE,!lay->active,
		       -1);
    lay = (Layer*)lay->next;
  }


  return(true);
}

bool gtk_ctrl_poll() {
  gdk_threads_enter();
  while(gtk_events_pending())
    gtk_main_iteration();
  gdk_threads_leave();
  return(true);
}
