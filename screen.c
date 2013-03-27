/* 
 * Copyright (c) 2003,2004  Daniel Bryan
 * All rights reserved.
 *
 * For more information see COPYRIGHT.
 */

#include <string.h>
#include <sys/time.h>

#include "common.h"
#include "screen.h"
#include "misc.h"
#include "path.h"
#include "color.h"

int winw,winh,mainww;
int mainwsbw; /* width of status bar */

/* create windows, draw border and title */
void scrn_draw() {
	int dev_null;
	
	if(scrn_border) {
		wborder(stdscr,0,0,0,0,0,0,0,0);
		mvwhline(stdscr,9,1,0,winw-2);
		wmove(stdscr,9,0);
		waddch(stdscr,ACS_LTEE);
		wmove(stdscr,9,winw-1);
		waddch(stdscr,ACS_RTEE);
	}
	
	mainw = newwin(8,winw-2,1,1);
	logw = newwin(winh-11,winw-2,10,1);
	overwrite(mainw, stdscr);
	overwrite(logw, stdscr);

	nonl();
	cbreak();
	noecho();
	refresh();
	nodelay(mainw, TRUE);
	keypad(mainw, TRUE);
	keypad(logw, TRUE);
	scrollok(logw, TRUE);
	getmaxyx(mainw, dev_null, mainww);
	mainwsbw = mainww - 2;
	
	wmove(mainw,0,0);
	wattron(mainw,A_BOLD);
	wprintw(mainw,"VCP %d.%d",VER_MAJ,VER_MIN);
	wattroff(mainw,A_BOLD);
	
	curs_set(0);
	refresh();	
	return;
}

/* check to see if the window has been re-sized */
int scrn_check() {
	int rwinh, rwinw;
	if(!use_curses)
		return 0;
	
	getmaxyx(stdscr, rwinh, rwinw);
	if(rwinh != winh || rwinw != winw) {
		winw = rwinw;
		winh = rwinh;
		/* TODO: check for valid screen size */
		delwin(mainw);
		delwin(logw);
		clear();
		scrn_draw();
		getmaxyx(mainw,rwinw,mainww);
		refresh();
		mainwsbw = mainww - 2;
		wrefresh(mainw);
		return 1;
	} else {
		return 0;
	}
}

void scrn_init() {

	if(tflag) {
		use_curses = 0;
		return;
	}
	
	initscr();
	getmaxyx(stdscr, winh, winw);
	if(winh < 11 || winw < 20) {
		endwin();
		use_curses = 0;
		logadds(LOG_ERR,"Screen size too small, %s", \
			"output to console",NULL);
		return;
	}

	use_curses = 1;
		
	if(has_colors() != TRUE)
		use_color = 0;
	if(use_color > 0) {
		start_color();
		use_default_colors();
		if(use_color == BLACK || use_color == BLACK2)
			init_pair(1, COLOR_BLACK,   -1);
		else if(use_color == RED || use_color == RED2)
			init_pair(1, COLOR_RED,     -1);
		else if(use_color == GREEN || use_color == GREEN2)	
			init_pair(1, COLOR_GREEN,   -1);
		else if(use_color == YELLOW || use_color == YELLOW2)
			init_pair(1, COLOR_YELLOW,  -1);
		else if(use_color == BLUE || use_color == BLUE2)
			init_pair(1, COLOR_BLUE,    -1);
		else if(use_color == MAGENTA || use_color == MAGENTA2)	
			init_pair(1, COLOR_MAGENTA, -1);
		else if(use_color == CYAN || use_color == CYAN2)
			init_pair(1, COLOR_CYAN,    -1);
		else if(use_color == WHITE || use_color == WHITE2)
			init_pair(1, COLOR_WHITE,   -1);
	}

	scrn_draw();
	logadds(LOG_MSG,"- Messages -",NULL,NULL);
	refresh();
	return;
}

void scrn_upd_file(char *src,char *dest) {

	if(!use_curses)
		return;
	
	wmove(mainw,2,0);
	wclrtoeol(mainw);
	wmove(mainw,2,0);
	wattron(mainw,A_BOLD);
	wprintw(mainw,"Copying: ");
	wattroff(mainw,A_BOLD);
	wprintw(mainw,"%s ",pathname(src));
	if(dest != NULL && strcmp(pathname(src),pathname(dest)) != 0) {
		wprintw(mainw,"-> %s ",pathname(dest));
	}
	coloron(mainw);
	waddch(mainw,'(');
	coloroff(mainw);
	wprintw(mainw,"%lu/%lu",curfile,totalfiles);
	coloron(mainw);
	waddch(mainw,')');
	coloroff(mainw);
		
	if(totalfiles != 1)
		scrn_setsb(7,(float)curfile / (float)totalfiles * \
			(float)mainwsbw);
	else
		scrn_setsb(7,0);
	
	if(totalfiles > 1 && curfile > 0) {
		wmove(mainw,7,(mainwsbw / 2) - 2);
		wprintw(mainw," %2.0f%% ", (float)((float)curfile / \
			(float)totalfiles * 100));
	}
	wrefresh(mainw);
	return;
}

void scrn_upd_part(unsigned long sizecpd,unsigned long size) {
	static unsigned long sizelast = 0;
	static unsigned long speed = 0;
	struct timeval now;
	static struct timeval last;
	unsigned int timer = 0;

	if(!use_curses)
		return;

	/* fail-safe */
	if(sizecpd > size)
		return;
		
	wmove(mainw,3,0);
	wclrtoeol(mainw);	
	wmove(mainw,3,0);
	wattron(mainw,A_BOLD);
	wprintw(mainw,"Copied: ");
	wattroff(mainw,A_BOLD);
	if(sizecpd < 1024)
		wprintw(mainw,"%lu B ",sizecpd);
	else if(sizecpd < 1048576)
		wprintw(mainw,"%1.2f kB ",(float)sizecpd/1024);
	else if(sizecpd < 1073741824)
		wprintw(mainw,"%1.2f mB ",(float)sizecpd/1048576);
	else
		wprintw(mainw,"%1.2f gB ",(float)sizecpd/1073741824);
	
	coloron(mainw);
	waddch(mainw,'/');
	coloroff(mainw);
	
	if(size < 1024)
		wprintw(mainw," %lu B",size);
	else if(size < 1048576)
		wprintw(mainw," %1.2f kB",(float)size/1024);
	else if(size < 1073741824)
		wprintw(mainw," %1.2f mB",(float)size/1048576);
	else
		wprintw(mainw," %1.2f gB",(float)size/1073741824);
	
	wmove(mainw,5,0);
	wclrtoeol(mainw);
	wmove(mainw,5,0);
	wattron(mainw,A_BOLD);
	wprintw(mainw,"ETR: ");
	wattroff(mainw,A_BOLD);
	
	gettimeofday(&now,NULL);
	if(sizelast > sizecpd)
		sizelast = 0;
	if((now.tv_sec - last.tv_sec) > 1) {
		speed = (float)(sizecpd - sizelast) / \
			(float)(now.tv_sec - last.tv_sec);
		gettimeofday(&last,NULL);
		sizelast = sizecpd;
	}
	if((size - sizecpd) < 1 || speed < 1) {
		wprintw(mainw,"? sec ");
	} else {
		timer = (size - sizelast) / (int)speed;
	
		if(timer > 3600) {
			wprintw(mainw,"%d hrs ",timer / 3600);
			timer -= ((timer / 3600) * 3600);
		}
		if(timer > 60) {
			wprintw(mainw,"%d min ",timer / 60);
			timer -= ((timer / 60) * 60);
		}
		wprintw(mainw,"%d sec ",timer);
	}
	
	coloron(mainw);
	waddch(mainw,':');
	coloroff(mainw);
	
	if(speed < 1024)
		wprintw(mainw,"%lu B/sec",speed);
	else if(speed < 1048576)
		wprintw(mainw,"%1.2f kB/sec",(float)speed/1024);
	else if(speed < 1073741824)
		wprintw(mainw,"%1.2f mB/sec",(float)speed/1048576);
	else
		wprintw(mainw,"%1.2f gB/sec",(float)speed/1073741824);

	coloron(mainw);
	waddch(mainw,' ');
	coloroff(mainw);
	
	if(totalfiles == 1) {
		scrn_setsb(7,(float)sizecpd / (float)size * (float)mainwsbw);
	}
	wmove(mainw,6,1);
	wclrtoeol(mainw);
	scrn_setsb(6,(float)sizecpd / (float)size * (float)mainwsbw);

	if(sizecpd > 0 && size > 0) {
		wmove(mainw,6,(mainwsbw / 2) -2);
		wprintw(mainw," %2.0f%% ",(float)((float)sizecpd / \
			(float)size * 100));
	}
	wrefresh(mainw);
	return;
}

/* set the status bar */
void scrn_setsb(int y, int x) {
	wmove(mainw,y,0);
	coloron(mainw);
	waddch(mainw,'[');
	coloroff(mainw);

	wattron(mainw,A_BOLD);
	while(x > 0) {
		waddch(mainw, '=');
		x--;
	}
	wattroff(mainw,A_BOLD);
	
	wmove(mainw,y,mainww-1);
	coloron(mainw);
	waddch(mainw,']');
	coloroff(mainw);
}

/* TODO: fix the lag, maybe up the r/w buf */
/* update screen in non-curses output */
void scrn_updtxt(unsigned long sizecpd,unsigned long size,int lastl) {
	int x = 0;
	int xo = 0;
	static int maxline = 0;
	
	struct timeval now;
	static struct timeval last;
	static unsigned long speed = 0;
	static unsigned long sizelast = 0;
	
	if(!vflag)
		return;
	
	if(sizecpd > 0) {
		x = (float)sizecpd / (float)size * 40;
		xo = 40 - x;
	} else {
		x = 0;
		xo = 40;
	}
	
	if(lastl) {
		/* clear line */
		fprintf(stderr,"\r");
		x = maxline + 41;
		while(x > 0) {
			fprintf(stderr," ");
			x--;
		}
		fprintf(stderr,"\r");
		maxline = 0;
		return;
	}

	fprintf(stderr,"\r[");
	while(x > 0) {
		fprintf(stderr,"=");
		x--;
	}
	while(xo > 0) {
		fprintf(stderr," ");
		xo--;
	}
	
	gettimeofday(&now,NULL);
	if(sizelast > sizecpd)
		sizelast = 0;
		
	if((now.tv_sec - last.tv_sec) > 1) {
		speed = (float)(sizecpd - sizelast) / \
			(float)(now.tv_sec - last.tv_sec);
		gettimeofday(&last,NULL);
		sizelast = sizecpd;
	}
	
	if(speed < 1024)
		x += fprintf(stderr,"] %lu B/sec",speed);
	else if(speed < 1048576)
		x += fprintf(stderr,"] %1.2f kB/sec",(float)speed/1024);
	else if(speed < 1073741824)
		x += fprintf(stderr,"] %1.2f mB/sec",(float)speed/1048576);
	else
		x += fprintf(stderr,"] %1.2f gB/sec",(float)speed/1073741824);

	if(sizecpd < 1024)
		x += fprintf(stderr," (%lu B",sizecpd);
	else if(size < 1048576)
		x += fprintf(stderr," (%1.2f kB",(float)sizecpd/1024);
	else if(size < 1073741824)
		x += fprintf(stderr," (%1.2f mB",(float)sizecpd/1048576);
	else
		x += fprintf(stderr," (%1.2f gB",(float)sizecpd/1073741824);
	
	x += fprintf(stderr,")    \b\b\b\b");
	
	if(x > maxline)
		maxline = x;
	
	fflush(stderr);
	return;
}
