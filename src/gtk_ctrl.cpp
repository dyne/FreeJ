/*  FreeJ
 *  (c) Copyright 2001-2003 Denis Roio aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * "$Id$"
 *
 */

#include <SDL/SDL.h>
#include <pthread.h>
#include <context.h>
#include <plugger.h>
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
static GladeXML *gtk;
static GtkWidget *wg;
static GtkTreeIter iter;

static GtkWidget *layer_list;
static GtkTreeStore *layer_store;
static GtkTreeIter layer_iter;


static GtkWidget *effect_list;
static GtkTreeStore *effect_store;
static GtkTreeIter effect_iter;

static GtkMenu *menu_effect;

/* direct pointer to FreeJ environment */
static Context *env;

static bool quit;

pthread_t _thread;
pthread_attr_t _attr;

/* layer's list model columns */
enum {
  LAYER_ACTIVE,
  LAYER_BLIT,
  LAYER_NAME,
  LAYER_FILE,
  LAYER_OBJ,
  LAYER_COLS
};

/* effect's list model columns */
enum {
  EFFECT_ACTIVE,
  EFFECT_NAME,
  EFFECT_OBJ,
  EFFECT_COLS
};

void on_fullscreen(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  SDL_WM_ToggleFullScreen(env->surf);
}
void on_about(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  env->osd->credits();
}
void do_add_layer(GtkWidget *widget, gpointer *data) {
  Layer *lay;
  char **sel = gtk_file_selection_get_selections(GTK_FILE_SELECTION(data));
  for(int c=0;sel[c];c++) {
    func("%s : creating %s",__FUNCTION__,sel[c]);
    lay = create_layer(sel[c]);
    if(lay) env->add_layer(lay);
    env->layers.sel(0); /* deselect others */
    lay->sel(true);
  }
}
void on_add_layer(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  /* create a file selector dialog */
  GtkWidget *fs = gtk_file_selection_new("Add a new Layer from file:");
  /* register callback when ok is pressed */
  g_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button),"clicked",
		   G_CALLBACK(do_add_layer), (gpointer)fs);
  /* destroy the fileselector when a button is pressed */
  g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button),"clicked",
			   G_CALLBACK(gtk_widget_destroy),(gpointer)fs);
  g_signal_connect_swapped(GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button),"clicked",
			   G_CALLBACK(gtk_widget_destroy),(gpointer)fs);
  gtk_file_selection_set_select_multiple(GTK_FILE_SELECTION(fs),true);
  /* show the widget */
  gtk_widget_show(fs);
}


void on_layer_active(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(laysel)
    laysel->active = 
      gtk_toggle_button_get_active
      ((GtkToggleButton*)widget);
}
void on_layer_delete(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(laysel) {
    laysel->rem();
    delete laysel;
    laysel = NULL;
  }
}

void on_layer_up(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(laysel) laysel->up(); }
void on_layer_down(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(laysel) laysel->down(); }
void on_blit_rgb(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(laysel) laysel->set_blit(1); }
void on_blit_red(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(laysel) laysel->set_blit(4); }
void on_blit_green(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(laysel) laysel->set_blit(3); }
void on_blit_blue(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(laysel) laysel->set_blit(2); }
void on_blit_add(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(laysel) laysel->set_blit(5); }
void on_blit_sub(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(laysel) laysel->set_blit(6); }
void on_blit_and(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(laysel) laysel->set_blit(7); }
void on_blit_or(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(laysel) laysel->set_blit(8); }
void on_osd(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  env->osd->active(); }
void on_overwrite(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  env->clear_all = !env->clear_all; }
void on_quit(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  quit = true; env->quit = true; }

/* =================== LAYER LIST */
void on_layer_select(GtkTreeSelection *sel, gpointer data) {
  func("%s(%p,%p)",__FUNCTION__,sel,data);
  GtkTreeModel *model;
  Layer *laysel;
  if(gtk_tree_selection_get_selected(sel,&model,&layer_iter)) {
    //    gtk_tree_selection_select_iter(sel,&layer_iter);
    gtk_tree_model_get(model,&layer_iter,LAYER_OBJ,&laysel,-1);
    if(laysel) {
      func("selected %s Layer %s (%p)",
	   laysel->getname(),laysel->get_filename(),laysel);
      env->layers.sel(0); /* deselect all */
      laysel->sel(true); /* select the one */
    }
  }
}
void on_effect_up(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(!laysel) return;
  Filter *filt = (Filter*)laysel->filters.selected();
  if(filt) filt->up();
}
void on_effect_down(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(!laysel) return;
  Filter *filt = (Filter*)laysel->filters.selected();
  if(filt) filt->down();
}
void on_effect_delete(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(!laysel) return;
  Filter *filt = (Filter*)laysel->filters.selected();
  if(filt) {
    filt->rem();
    filt->inuse = false;
    filt->initialized = false;
  }
}
void on_effect_active(GtkWidget *widget, gpointer *data) {
  func("%s(%p,%p)",__FUNCTION__,widget,data);
  Layer *laysel = (Layer*) env->layers.selected();
  if(!laysel) return;
  Filter *filt = (Filter*)laysel->filters.selected();
  if(filt)
    filt->active = gtk_toggle_button_get_active
      ((GtkToggleButton*)widget);
}

void init_layer_list() {
  
  GtkCellRenderer *rend;
  GtkTreeViewColumn *col;

  layer_store =  gtk_tree_store_new(LAYER_COLS,
				    G_TYPE_BOOLEAN,
				    G_TYPE_STRING, /* name */
				    G_TYPE_STRING, /* blit */
				    G_TYPE_STRING, /* file */
				    G_TYPE_POINTER); /* object pointer */

  layer_list = glade_xml_get_widget(gtk,"treeview_layer");
  /* register the model on the list view */
  gtk_tree_view_set_model((GtkTreeView*)layer_list,GTK_TREE_MODEL(layer_store));
  /* we are discarding the reference to the model now 
     g_object_unref(G_OBJECT(layer_store)); */

  /* setup selection handler */
  {
    GtkTreeSelection *layer_select;
    layer_select = gtk_tree_view_get_selection(GTK_TREE_VIEW(layer_list));
    gtk_tree_selection_set_mode(layer_select, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(layer_select), "changed",
		     G_CALLBACK(on_layer_select), NULL);
  }
  
  /* initialize tree view columns and renderers */
  rend = gtk_cell_renderer_toggle_new();
  col = gtk_tree_view_column_new_with_attributes
    ("ON",rend,"active",LAYER_ACTIVE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(layer_list),col);
  /* and why this doesn't works?
  g_object_set(G_OBJECT(rend),"activatable",true);
  g_signal_connect(G_OBJECT(rend),"toggled",
		   G_CALLBACK(on_layer_active),NULL);
  */

  rend = gtk_cell_renderer_text_new();
  g_object_set(G_OBJECT(rend),"background","orange",NULL);
  col = gtk_tree_view_column_new_with_attributes
    ("Name",rend,"text",LAYER_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(layer_list),col);

  rend = gtk_cell_renderer_text_new();
  g_object_set(G_OBJECT(rend),"background","cyan",NULL);
  col = gtk_tree_view_column_new_with_attributes
    ("Blit",rend,"text",LAYER_BLIT,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(layer_list),col);  

  rend = gtk_cell_renderer_text_new();
  col = gtk_tree_view_column_new_with_attributes
    ("File",rend,"text",LAYER_FILE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(layer_list),col);  

}
void update_layer_list() {
  /* fill up the list of layers allready loaded */
  gtk_tree_store_clear(layer_store);
  Layer *lay = (Layer*)env->layers.begin();
  while(lay) {
    lay->lock();
    gtk_tree_store_append(layer_store,&iter,NULL);
    gtk_tree_store_set(layer_store,&iter,
		       LAYER_ACTIVE,lay->active,
		       LAYER_NAME,lay->getname(),
		       LAYER_BLIT,lay->get_blit(),
		       LAYER_FILE,lay->get_filename(),
		       LAYER_OBJ,lay,
		       -1);
    lay->unlock();
    lay = (Layer*)lay->next;
  }
}







  /* =================== EFFECT LIST */
void on_effect_select(GtkTreeSelection *sel, gpointer data) {
  func("%s(%p,%p)",__FUNCTION__,sel,data);
  GtkTreeModel *model;
  Filter *effsel;
  if(gtk_tree_selection_get_selected(sel,&model,&effect_iter)) {
    //    gtk_tree_selection_select_iter(sel,&layer_iter);
    gtk_tree_model_get(model,&effect_iter,EFFECT_OBJ,&effsel,-1);
    if(effsel) {
      Layer *laysel = (Layer*) env->layers.selected();
      if(!laysel) return;
      func("selected effect %s on layer %s",
	   effsel->getname(), laysel->getname());
      laysel->filters.sel(0); /* deselect all */
      effsel->sel(true); /* select the one */
    }
  }
}
void init_effect_list() {


  GtkCellRenderer *rend;
  GtkTreeViewColumn *col;
  
  effect_store = gtk_tree_store_new(EFFECT_COLS,
			     G_TYPE_BOOLEAN, /* name */
			     G_TYPE_STRING, /* on */
			     G_TYPE_POINTER); /* object */
  effect_list = glade_xml_get_widget(gtk,"treeview_effect");
  /* register the model on the list view */
  gtk_tree_view_set_model((GtkTreeView*)effect_list,GTK_TREE_MODEL(effect_store));

  { /* setup selection handler */
    GtkTreeSelection *effect_select;
    effect_select = gtk_tree_view_get_selection(GTK_TREE_VIEW(effect_list));
    gtk_tree_selection_set_mode(effect_select, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(effect_select), "changed",
		     G_CALLBACK(on_effect_select), NULL);
  }

  /* we can discard the reference to the model now 
     g_object_unref(G_OBJECT(store));*/
  /* initialize tree view columns and renderers */
  rend = gtk_cell_renderer_toggle_new();
  col = gtk_tree_view_column_new_with_attributes
    ("ON",rend,"active",EFFECT_ACTIVE,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(effect_list),col);
  rend = gtk_cell_renderer_text_new();
  col = gtk_tree_view_column_new_with_attributes
    ("Name",rend,"text",EFFECT_NAME,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(effect_list),col);
}


void update_effect_list() {
  gtk_tree_store_clear(effect_store);
  Layer *laysel = (Layer*) env->layers.selected();
  if(!laysel) return;
  Filter *filt = (Filter*)laysel->filters.begin();
  while(filt) {
    gtk_tree_store_append(effect_store,&iter,NULL);
    gtk_tree_store_set(effect_store,&iter,
		       EFFECT_ACTIVE,filt->active,
		       EFFECT_NAME,filt->getname(),
		       EFFECT_OBJ,filt,
		       -1);
    filt = (Filter*)filt->next;
  }
}
void on_add_effect(char *name) {
  gtk_menu_set_active(menu_effect,0);
  Layer *laysel = (Layer*) env->layers.selected();
  Filter *filt;
  if(!laysel) {
    error("no layer selected for effect %s",name); return; }
  /* TODO plugin selection by name inside plugger
     i don't do this now, will develop LiViDO and come back later */
  for(int c=0; (filt = (Filter*)env->plugger->plugs[c]) ; c++) {
    if(filt->inuse) continue;
    if(strcasecmp(filt->getname(),name)==0)
      laysel->add_filter(filt);
  }
}
void init_effect_menu() {
  GtkOptionMenu *option_menu_effect;
  GtkWidget *item;

  option_menu_effect = (GtkOptionMenu*) glade_xml_get_widget(gtk,"menu_effect");
  menu_effect = (GtkMenu*)gtk_menu_new();
  gtk_menu_set_title(menu_effect,"Add effect");
  item = gtk_menu_item_new_with_label("Add effect");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_effect),item);
  for(int c=0;env->plugger->plugs[c];c++) {
    item = gtk_menu_item_new_with_label(env->plugger->plugs[c]->getname());
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_effect),item);
    g_signal_connect_swapped(G_OBJECT(item),"activate",
			     G_CALLBACK(on_add_effect),
			     (gpointer)env->plugger->plugs[c]->getname());
  }
  gtk_option_menu_set_menu(option_menu_effect,(GtkWidget*)menu_effect);
}

void *gtk_run(void *arg) {
  while(!quit) {
    update_layer_list();
    update_effect_list();
    gdk_threads_enter();
    while(gtk_events_pending())
      gtk_main_iteration();
    gdk_threads_leave();
    jsleep(0,100);
  }
  return(NULL);
}

void gtk_ctrl_quit() {
  quit = true;
  jsleep(0,500);
  if(pthread_attr_destroy(&_attr) == -1)
    error("error destroying POSIX thread attribute");
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
  CONNECT("quit","activate",on_quit);
  CONNECT("main_win","destroy",on_quit);

  CONNECT("button_layer_up","clicked",on_layer_up);
  CONNECT("button_layer_down","clicked",on_layer_down);
  CONNECT("button_layer_delete","clicked",on_layer_delete);
  CONNECT("checkbutton_layer_active","clicked",on_layer_active);
  CONNECT("blit_rgb","activate",on_blit_rgb);
  CONNECT("blit_red","activate",on_blit_red);
  CONNECT("blit_green","activate",on_blit_green);
  CONNECT("blit_blue","activate",on_blit_blue);
  CONNECT("blit_add","activate",on_blit_add);
  CONNECT("blit_sub","activate",on_blit_sub);
  CONNECT("blit_and","activate",on_blit_and);
  CONNECT("blit_or","activate",on_blit_or);

  CONNECT("button_effect_up","clicked",on_effect_up);
  CONNECT("button_effect_down","clicked",on_effect_down);  
  CONNECT("button_effect_delete","clicked",on_effect_delete);
  CONNECT("checkbutton_effect_active","clicked",on_effect_active);

  init_layer_list();
  init_effect_list();
  init_effect_menu();

  quit = false;
  if(pthread_attr_init (&_attr) == -1)
    error("error initializing POSIX thread attribute");
  /* sets the thread as detached
     see: man pthread_attr_init(3) */
  pthread_attr_setdetachstate(&_attr,PTHREAD_CREATE_DETACHED);
  pthread_create(&_thread, &_attr, gtk_run, NULL);
  return true;
}
