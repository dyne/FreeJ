#ifndef __GTK_CTRL_H__
#define __GTK_CTRL_H__

class Context;

bool gtk_ctrl_init(Context *nenv, int *argc, char **argv);
void gtk_ctrl_quit();

#endif
