/* 
 * Copyright (c) 2003,2004  Daniel Bryan
 * All rights reserved.
 *
 * For more information see COPYRIGHT.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <fts.h>
#include <curses.h>

#include "log.h"

#define VER_MAJ 2
#define VER_MIN 2

/* destination types */
#define T_FILE 0 /* FILE -> FILE             */
#define T_DIR  1 /*      -> DIR              */
#define T_NED  2 /* DIR  -> NON-EXISTENT DIR */

WINDOW *mainw;
WINDOW *logw;

struct dest_new {
	char *opath;
	char path[MAXPATHLEN+1];
};
extern struct dest_new dest;

extern unsigned long totalfiles;
extern unsigned long curfile;
extern unsigned long goodcp;

extern int Rflag,vflag,Hflag,Pflag,\
	Lflag,fflag,iflag,tflag,pflag, \
	dflag,hflag,mflag,Vflag,Iflag, \
	nflag,uflag;


#define SCRN_LEAVE    0
#define SCRN_SUMMARY  1
#define SCRN_KEYWAIT  2
/* 
 * scrn_state:
 * 0 - leave (just leave)
 * 1 - summary (summary with errors)
 * 2 - keywait (wait for a key press then leave)
 */
extern int scrn_state;
extern int scrn_border;

extern int use_color;
extern int use_curses;
extern int ret;
extern int buf_size;
extern char *databuf;

int copyall(char *args[],int fts_opt,int type);
