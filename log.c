/* 
 * Copyright (c) 2003,2004  Daniel Bryan
 * All rights reserved.
 *
 * For more information see COPYRIGHT.
 */

#include <stdarg.h>
#include <stdlib.h>

#include "common.h"
#include "misc.h"
#include "screen.h"
#include "color.h"

int logaddi(int code, char *base, int var, int var2) {
	
	if(code == LOG_VRB && !vflag)
		return 1;
	if(use_curses) {
		wprintw(logw, base, var, var2);
		wprintw(logw, "\n");
		wrefresh(logw);
	} else {
		printf(base, var, var2);
		printf("\n");
		fflush(stdout);
	}	
	return 0;
}

int logadds(int code, char *base, char *var, char *var2) {
	if(code == LOG_VRB && !vflag)
		return 1;
	if(use_curses) {
		wprintw(logw, base,var,var2);
		wprintw(logw, "\n");
		wrefresh(logw);
	} else {
		printf(base, var, var2);
		printf("\n");
		fflush(stdout);
	}	
	return 0;
}

int logget(char *name) {
	char c,decoy;
	if(use_curses) {
		if(Iflag)
			wprintw(logw, "Overwrite %s ? [Y]/N ",name);
		else
			wprintw(logw, "Overwrite %s ? Y/[N] ",name);
		wrefresh(logw);
		c = wgetch(logw);
		if(c == '\n' || c == '\r') {
			wprintw(logw,"\n");
			wrefresh(logw);
			if(Iflag)
				return 0;
			else
				return 1;
		}
		wprintw(logw,"%c\n",c);
		wrefresh(logw);
	} else {
		if(Iflag)
			printf("Overwrite %s ? [Y]/N ",name);
		else
			printf("Overwrite %s ? Y/[N] ",name);
		c = decoy = getchar();
		while(decoy != '\n' && decoy != EOF)
			decoy = getchar();
	}
	if(c == '\n' || c == '\r') {
		if(Iflag)
			return 0;
		else
			return 1;
	}
	if(c == 'y' || c == 'Y')
		return 0;
	else
		return 1;
}
