/* 
 * Copyright (c) 2003,2004  Daniel Bryan
 * All rights reserved.
 *
 * For more information see COPYRIGHT.
 */
 
void scrn_draw();
int scrn_check();
void scrn_init();
void scrn_upd_file(char *src,char *dest);
void scrn_upd_part(unsigned long sizecpd,unsigned long size);
void scrn_setsb(int y, int x);
void scrn_updtxt(unsigned long sizecpd,unsigned long size,int lastl);
