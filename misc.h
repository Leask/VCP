/* 
 * Copyright (c) 2003,2004  Daniel Bryan
 * All rights reserved.
 *
 * For more information see COPYRIGHT.
 */

int statit(char *path,struct stat *st,int cmdline);
int setperm(char *src,struct stat *st);
void usage();
void done(int i);
void exit_pre();
void check_iloop(char *args[], char *test);
void print_error(char *src, int errno_);
void flags_print();
int cntfiles(char *args[],int fts_opt);
int conf_read(int path);
int flag_set(int flag, int value);
int flag_setc(char c, int value);
