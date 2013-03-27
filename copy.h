/* 
 * Copyright (c) 2003,2004  Daniel Bryan
 * All rights reserved.
 *
 * For more information see COPYRIGHT.
 */
 
int copylink(char *src, char *dest);
int copyfifo(char *destfile, struct stat *st);
int copyspecial(char *destfile, struct stat *st);
int copyfile(char *srcfile, char *destfile, struct stat *st);
