/* 
 * Copyright (c) 2003,2004  Daniel Bryan
 * All rights reserved.
 *
 * For more information see COPYRIGHT.
 */

#include "common.h"
#include "color.h"

void coloron(WINDOW *win) {	
	if(use_color == 0)
		return;
	if(use_color >= USE_BRIGHT)
		wattron(win,A_BOLD);
	else
		wattroff(win,A_BOLD);
	wattron(win, COLOR_PAIR(1));
	return;
}
void coloroff(WINDOW *win) {	
	if(use_color == 0)
		return;
	wattroff(win, COLOR_PAIR(1));
	if(use_color >= USE_BRIGHT)
		wattroff(win,A_BOLD);
	return;
}
