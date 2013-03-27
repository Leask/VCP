/* 
 * Copyright (c) 2003,2004  Daniel Bryan
 * All rights reserved.
 *
 * For more information see COPYRIGHT.
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "common.h"
#include "screen.h"
#include "misc.h"
#include "copy.h"
#include "path.h"

int copylink(char *src, char *dest) {
	
	char buf[MAXPATHLEN];
	int exists = 0;
	int i;
	struct stat st2;
		
	if(stat(dest,&st2) != -1)
		exists=1;
	
	i = readlink(src,buf,sizeof(buf)-1);
	if(i < 0) {
		logadds(LOG_ERR,"%s: Bad link",src,NULL);
		return 1;
	}
	buf[i] = '\0';
	
	if(use_curses) {
		scrn_upd_file(src,dest);
	} else if(vflag) {
		fprintf(stderr,"\r%lu/%lu - %s -> %s\n",curfile,totalfiles,src,dest);
	}
		
	if(exists) {
		if(unlink(dest) == -1){
			logadds(LOG_ERR,"%s: Can not overwrite link",dest,NULL);
			return 1;
		}
	}
	if(symlink(buf,dest) == -1){
		logadds(LOG_ERR,"%s: Can not create link",dest,NULL);
		return 1;
	}

	if(use_curses)
		logadds(LOG_VRB,"%s: Created",dest,NULL);
	goodcp++;
	
	return 0;
}

int copyfifo(char *destfile, struct stat *st) {
	struct stat tmp;
	
	if(use_curses) {
		scrn_upd_file(destfile,NULL);
	} else if(vflag) {
		fprintf(stderr,"\r%lu/%lu - %s\n",curfile,totalfiles,destfile);
	}	
	if(stat(destfile,&tmp) != -1) {
		if(unlink(destfile))
			return 1;
	}
	if(mkfifo(destfile,st->st_mode) == -1) {
		logadds(LOG_ERR,"%s: Can not create fifo file",destfile,NULL);
		return 1;
	}
	setperm(destfile,st);
	if(use_curses)
		logadds(LOG_VRB,"%s: Created",destfile,NULL);
	goodcp++;
	return 0;
}

int copyspecial(char *destfile, struct stat *st) {
	struct stat tmp;
	
	if(use_curses) {
		scrn_upd_file(destfile,NULL);
	} else if(vflag) {
		fprintf(stderr,"\r%lu/%lu - %s\n",curfile,totalfiles,destfile);
	}
	if(stat(destfile,&tmp) != -1) {
		if(unlink(destfile))
			return 1;
	}
	if(mknod(destfile,st->st_mode,st->st_rdev)) {
		logadds(LOG_ERR,"%s: Can not create file",destfile,NULL);
		return 1;
	}
	setperm(destfile,st);
	if(use_curses)
		logadds(LOG_VRB,"%s: Created",destfile,NULL);
	goodcp++;
	return 0;
}

#define MAX_BUF 204800 /* default buffer size */

int copyfile(char *srcfile, char *destfile, struct stat *st) {
	
	int f, f2;
	struct stat st2;
	int rcnt = 0;
	int rest = 0;
	int wcnt = 0;
	char *p  = NULL;
	unsigned long sizecpd, size;
	struct timeval timea;
		
	/* 0.1 seconds 
	ts.tv_nsec = 100000000;
	ts.tv_sec = 0;*/
	timea.tv_sec = 0;
	timea.tv_usec = 100;
	
	if(use_curses) {
		scrn_upd_file(srcfile,destfile);
	} else if(vflag) {
		fprintf(stderr,"\r%lu/%lu - %s\n",curfile,totalfiles,srcfile);
	}
	if((f = open(srcfile, O_RDONLY)) == -1) {
		print_error(srcfile,errno);
		return 1;
	}
	if(stat(destfile,&st2) != -1) {
		if(iflag || Iflag) {
			if(logget(destfile))
				return 1;
		}
		if(nflag) {
			logadds(LOG_VRB,"%s: file has not been overwritten",destfile,NULL);
			return 0;
		}
		if(fflag) {
			unlink(destfile);
			f2 = open(destfile, O_WRONLY | O_CREAT | O_TRUNC,\
				st->st_mode & ~(S_ISUID | S_ISGID));
		} else
			f2 = open(destfile, O_WRONLY | O_TRUNC,0);
	} else {
		f2 = open(destfile, O_WRONLY | O_CREAT | O_TRUNC,\
			st->st_mode & ~(S_ISUID | S_ISGID));
	}
	if(f2 == -1) {
		print_error(destfile,errno);
		return 1;
	}
	size = st->st_size;
	sizecpd = 0;
	
	if(size < 1) {
		logadds(LOG_VRB,"%s: file size is 0",srcfile,NULL);
	} else {
		if(buf_size < 1)
			buf_size = MAX_BUF;
		if(databuf == NULL) {
			if((databuf = (char *)malloc(buf_size)) == NULL) {
				exit_pre();
				fprintf(stderr,"Can not allocate %d bytes of memory\n",buf_size);
				exit(1);
			}
		}
		while ((rcnt = read(f,databuf,buf_size)) > 0) {
			rest = rcnt;
			p = databuf;
			wcnt = 0;
			for(;;) {
				p += wcnt;
				rest -= wcnt;
				wcnt = write(f2,p,rest);
				if(rest <= wcnt || wcnt <= 0)
					break;
				
			}
			if(rest != wcnt) {
				logadds(LOG_ERR,"%s: Error writing file",destfile,NULL);
				return 1;
			}
			
			sizecpd += rcnt;
			if(use_curses)
				scrn_upd_part(sizecpd,size);
			else
				scrn_updtxt(sizecpd,size,0);
			if(dflag)
				select(0,NULL,NULL,NULL,&timea);
		}
		if(use_curses) {
			scrn_upd_part(sizecpd,size);		
		} else {
			scrn_updtxt(sizecpd,size,1);
		}
	}
	
	close(f);
	close(f2);
	
	if(rcnt < 0) {
		logadds(LOG_ERR,"%s: Error reading file",srcfile,NULL);
		return 1;
	}
	setperm(destfile,st);
	
	goodcp++;
	if(use_curses)
		logadds(LOG_VRB,"%s: File copied",pathname(srcfile),NULL);
	return 0;
}
