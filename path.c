/* 
 * Copyright (c) 2003,2004  Daniel Bryan
 * All rights reserved.
 *
 * For more information see COPYRIGHT.
 */

#include <string.h>

#include "common.h"
#include "path.h"

/*
possibilities:
	root  foo  = root/foo
	root  /foo = root/foo
	root/ foo  = root/foo
	root/ /foo = root/foo
*/
int pathdadd(char *master,char *dir) {
	int end = strlen(master)-1;
	
	if(dir == NULL)
		return 0;
	if(pathchsize(master,dir))
		return 1;
	if(master[end] == '/' && *dir == '/')
		dir++;
	else if(master[end] != '/' && *dir != '/') {
		if(master != NULL)
			strcat(master,"/");
	}
	strcat(master,dir);
	if(master[strlen(master)-1] == '/')
		master[strlen(master)-1] = '\0';
	return 0;
}

/* checks string(s) if they are larger then MAXPATHLEN */
int pathchsize(char *path, char *path2) {
	int i = 0;
	int d = 0;
	
	if(path == path2) {
		i = strlen(path);
		if(i<(MAXPATHLEN - 1))
			return 0;
	} else {
		if(path != NULL)
			i = strlen(path);
		if(path2 != NULL)
			d = strlen(path2);
	
		if((i+d)<(MAXPATHLEN - 1))
			return 0;
	}
	pathlong(path,path2);
	return 1;	
}

/* print 20 characters from the string(s),
   verybigstringfoobarr ... 123456789endofstring */
int pathlong(char *path,char *path2) {
	char buf[21],buf2[21];
	int x, l;
	int i = 0;
	int d = 0;
	
	if(path != NULL) {
		if(path == path2)
			i = d = strlen(path);
		else
			i = strlen(path);
	}
	if(path2 != NULL)
		d = strlen(path2);
	
	if(i<21 && i>0)
		strcpy(buf,path);
	else if(i>0){
		for(x=0;x<20;x++)
			buf[x] = path[x+1];
		buf[20] = '\0';
	}
		
	if(d<21 && d>0)
		strcpy(buf2,path2);
	else if(d>0){
		l = strlen(path2) - 20;
		if(path == path2) {
			for(x=0;x<20;x++)
				buf2[x] = path2[x+l];
			buf2[20] = '\0';
		} else {
			for(x=0;x<20;x++)
				buf2[x] = path[x+l];
			buf2[20] = '\0';
		}		
	}
	
	logadds(LOG_ERR,"%s ... %s: Path too long, not copied",buf,buf2);
	return 1;
}

/* returns the filename as int */
int pathnamed(char *path) {
	char *p;
	p = strrchr(path,'/');
	if(path[strlen(path)-1] == '/') {
		p--;
		p = strrchr(path,'/');
	}
	if(p == NULL)
		return 0;
	return p - path;
}

/* returns a pointer to the filename */
char *pathname(char *path) {
	char *p;
	if(path == NULL)
		return path;
	p = strrchr(path,'/');
	if(p == NULL)
		return path;
	p++;
	return p;
}
