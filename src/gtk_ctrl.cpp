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


Context *env;
GladeXML *gtk;
GtkWidget *wg;


void on_fullscreen(GtkWidget *widget, gpointer *data) {
  SDL_WM_ToggleFullScreen(env->surf);
}

void on_about(GtkWidget *widget, gpointer *data) {
  env->osd->credits();
}


bool gtk_ctrl_init(Context *nenv, int *argc, char **argv) {
  env = nenv;
  gtk_init(argc, &argv);
  gtk = glade_xml_new("../freej-gtk2.glade",NULL,NULL);

  /* connect signal handlers 
     glade_xml_signal_autoconnect(gtk); */

  /* signal to glib we're going to use threads */
  g_thread_init(NULL);
  
  return(true);
  /*
  wg = glade_xml_get_widget(gtk, "fullscreen");
  if(wg) g_signal_connect((gpointer) wg, "activate",
			  G_CALLBACK(on_fullscreen),NULL);
  */
  CONNECT("fullscreen","activate",on_fullscreen);
  CONNECT("about","activate",on_about);
  //  gtk_connect("fullscreen","activate",on_fullscreen);
  //  gtk_connect("about","activate",on_about);
}

bool gtk_ctrl_poll() {
  gdk_threads_enter();
  while(gtk_events_pending())
    gtk_main_iteration();
  gdk_threads_leave();
  return(true);
}
