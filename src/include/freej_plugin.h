#ifndef __freej_plugin_h__
#define __freej_plugin_h__

static char *name;
static char *author;
static char *info;
static int version;
static int bpp;

char *getname() { return name; };
char *getauthor() { return author; };
char *getinfo() { return info; };
int getversion() { return version; };
int getbpp() { return bpp; };

/* INTERNAL =================================================
   this counts how many times a plugin is instantiated
   it is handled into the Plugin:: open() and close() methods */
int dlcount = 0;

/* here we hardcode maximum instances permitted by every plugin */
int maxinstances = 1;

int dlinc() { 
  if(dlcount<maxinstances) {
    dlcount++; return(dlcount);
  } else return(-1); };
int dldec() { dlcount--; return(dlcount); };
/* ========================================================== */

#endif
