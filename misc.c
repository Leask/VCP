/* 
 * Copyright (c) 2003,2004  Daniel Bryan
 * All rights reserved.
 *
 * For more information see COPYRIGHT.
 */
 
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#if defined(__linux__)
#include <utime.h>
#endif

#include "common.h"
#include "misc.h"
#include "screen.h"
#include "path.h"
#include "color.h"
#include "screen.h"

int statit(char *path,struct stat *st,int cmdline) {
	if(Rflag) {
		if(pflag || ((Hflag) && !cmdline))
			return (lstat(path,st)); /* do not follow symbolic links */
	}
	return (stat(path,st)); /* follow symbolic links */
}

int setperm(char *src,struct stat *st) {
	struct stat tmp;
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)	
	static struct timeval timea[2];
#endif
#if defined(__linux__)
	struct utimbuf timea;
#endif
	if(!pflag)
		return 1;

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
	TIMESPEC_TO_TIMEVAL(&timea[0],&st->st_atimespec);
	TIMESPEC_TO_TIMEVAL(&timea[1],&st->st_mtimespec);
	if(utimes(src,timea) == -1) {
		logadds(LOG_ERR,"utimes: %s: %s",src,strerror(errno));
	}
#endif

#if defined(__linux__)
	timea.actime = st->st_atime;
	timea.modtime = st->st_mtime;
	if(utime(src,&timea) == -1) {
		logadds(LOG_ERR,"utime: %s: %s",src,strerror(errno));
	}
#endif

	tmp.st_mode &= (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO);
	
	if(st->st_uid != tmp.st_uid || st->st_gid != tmp.st_gid) {
		if(chown(src,st->st_uid,st->st_gid) == -1 && errno != EPERM) {
			logadds(LOG_ERR,"chown: %s: %s",src,strerror(errno));
		}
	}
	
	if(chmod(src,st->st_mode) == -1) {
		logadds(LOG_ERR,"chmod: %s: %s",src,strerror(errno));
	}
	
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
	if(chflags(src,st->st_flags) == -1) {
		logadds(LOG_ERR,"chflags: %s: %s",src,strerror(errno));
	}
#endif

	return 0;
}

void done(int i) {
	logadds(LOG_MSG,"",NULL,NULL);
	
	if(use_curses && scrn_state == SCRN_SUMMARY) {
		endwin();
		use_curses = 0;
	}
	if(mflag) {
		if(totalfiles / i == 1 && i == 1) {			
			logaddi(LOG_MSG,"%d file copied %d time",totalfiles / i,i);
			logaddi(LOG_MSG,"%d failed",totalfiles-goodcp,0);
		} else if(totalfiles / i == 1) {
			logaddi(LOG_MSG,"%d file copied %d times",totalfiles / i,i);
			logaddi(LOG_MSG,"%d failed",totalfiles-goodcp,0);
		} else if(i == 1) {
			logaddi(LOG_MSG,"%d files copied %d time",totalfiles / i,i);
			logaddi(LOG_MSG,"%d failed",totalfiles-goodcp,0);
		} else {
			logaddi(LOG_MSG,"%d files copied %d times",totalfiles / i,i);
			logaddi(LOG_MSG,"%d failed",totalfiles-goodcp,0);
		}
	} else {
		if(goodcp == 1) {
			logaddi(LOG_MSG,"%d file copied, %d failed",goodcp,totalfiles-goodcp);
		} else {
			logaddi(LOG_MSG,"%d files copied, %d failed",goodcp,totalfiles-goodcp);
		}
	}
	
	if(use_curses && scrn_state == SCRN_KEYWAIT)
		wgetch(logw);
	
	exit_pre();
	exit(ret);
}

void exit_pre() {
	if(databuf != NULL)
		free(databuf);
	if(use_curses)
		endwin();
	return;
}

void usage() {
	logadds(LOG_MSG,"",NULL,NULL);
	logadds(LOG_MSG,"usage:",NULL,NULL);
	logadds(LOG_MSG,"options %s %s", \
		"[-vdpVmhtu] [-I | -i | -f | -n] ", \
		"[-R [-P | -H | -L]] [-b bytes]");
	logadds(LOG_MSG,"vcp [options] src dest",NULL,NULL);
	logadds(LOG_MSG,"vcp [options] src1 src2 .. destdir",NULL,NULL);
	logadds(LOG_MSG,"vcp [options -m] src dest1 dest2 ..",NULL,NULL);
	logadds(LOG_MSG,"'vcp -h' for option help",NULL,NULL);
	exit_pre();
	exit(1);
}

void check_iloop(char *args[], char *test) {
	int i = 0;
	while(args[i] != NULL) {
		if((strcmp(args[i],test) == 0) || (strcmp(args[i],".") == 0)) {
			logadds(LOG_ERR,"Warning: possible infinite loop (%s)",args[i],NULL);
		}
		i++;
	}
	return;	
}

void print_error(char *src, int errno_) {
	if(errno_ == ENAMETOOLONG)
		pathlong(src,src);
	else
		logadds(LOG_ERR,"%s: %s",src,strerror(errno_));
	return;
}

void flags_print() {
	char buf[15];
	
	buf[0] = '\0';
	if(vflag)
		strcat(buf,"v");
	if(tflag)
		strcat(buf,"t");
	if(dflag)
		strcat(buf,"d");
	if(Iflag)
		strcat(buf,"I");
	if(iflag)
		strcat(buf,"i");
	if(fflag)
		strcat(buf,"f");
	if(nflag)
		strcat(buf,"n");
	if(Rflag)
		strcat(buf,"R");
	if(Hflag)
		strcat(buf,"H");
	if(Pflag)
		strcat(buf,"P");
	if(Lflag)
		strcat(buf,"L");
	if(mflag)
		strcat(buf,"m");		
	if(uflag)
		strcat(buf,"u");
	logadds(LOG_MSG,"Flags -%s",buf,NULL);
	return;
}

int cntfiles(char *args[],int fts_opt) {
	int total = 0;
	FTS *ftsd;
	FTSENT *ftsf;

	if((ftsd = fts_open(args,fts_opt,0)) == NULL)
		return 0;
	
	while((ftsf = fts_read(ftsd)) != NULL) {
		switch(ftsf->fts_info) {
			/* count the file even if invalid */
			case FTS_F:
			case FTS_NS:
			case FTS_NSOK:
			case FTS_SL:
			case FTS_SLNONE:
			case FTS_DEFAULT:
				if(!ftsf->fts_number) {
					ftsf->fts_number = 1;
					total++;
				}
		}
	}
	return total;
}

#define CONF_SIZE 64

int conf_readval(int file, char *buf) {
	int i;	
	
	for(i=0;i<CONF_SIZE;i++) {
		*(buf + i) = '\0';
	}
	
	for(i=0;i<CONF_SIZE;i++) {
		if(read(file,(buf + i),1) < 1)
			return -1;
		if(*(buf + i) == '\n') {
			*(buf + i) = '\0';
			return 0;
		}
		if(*(buf + i) == ' ' || *(buf + i) == '"') {
			i--;
			continue;
		}		
	}
	*(buf + CONF_SIZE - 1) = '\0';
	return 0;
}
	

int conf_read(int path) {

	int file;
	char buf[CONF_SIZE];
	int i,x;
	char *home;
	char *fullhome;
	
	char colors[17][9] = {
		{ "green2\0"   },
		{ "white2\0"   },
		{ "blue2\0"    },
		{ "red2\0"     },
		{ "magenta2\0" },
		{ "cyan2\0"    },
		{ "black2\0"   },
		{ "yellow2\0"  },
		
		{ "green\0"    },
		{ "white\0"    },
		{ "blue\0"     },
		{ "red\0"      },
		{ "magenta\0"  },
		{ "cyan\0"     },
		{ "black\0"    },
		{ "yellow\0"   },
		
		{ "off\0" },
	};
	int colors_d[] = {
		GREEN2,
		WHITE2,
		BLUE2,
		RED2,
		MAGENTA2,
		CYAN2,
		BLACK2,
		YELLOW2,
		
		GREEN,
		WHITE,
		BLUE,
		RED,
		MAGENTA,
		CYAN,
		BLACK,
		YELLOW,
		
		0,					
	};
	int xmax = 17;
	x = 0;
	
	if(path) {
		if((fullhome = (char *)malloc(14)) == NULL) {
			return -1;
		}		
		*fullhome = '\0';
		strcat(fullhome,"/etc/vcp.conf");
	
	} else {
		if((home = getenv("HOME")) == NULL) {
			return -1;
		}
		if((fullhome = (char *)malloc(strlen(home) + 6)) == NULL) {
			return -1;
		}
		*fullhome = '\0';
		strcat(fullhome,home);
		strcat(fullhome,"/.vcp");
	}
	
	if((file = open(fullhome,O_RDONLY)) == -1) {
		return -1;
	}
	free(fullhome);

	for(i=0;i<CONF_SIZE;i++) {
		buf[0] = '\0';
	}	
	while(conf_readval(file,buf) != -1) {
		if(strncmp(buf,"color=",6) == 0) {
			for(x=0;x<xmax;x++) {
				if(strcmp((buf + 6),colors[x]) == 0) {
					use_color = colors_d[x];
					break;
				}
			}
		}
		if(strncmp(buf,"screen=",7) == 0) {
			if(strcmp((buf + 7),"leave") == 0) {
				scrn_state = SCRN_LEAVE;
			} else if(strcmp((buf + 7),"summary") == 0) {
				scrn_state = SCRN_SUMMARY;
			} else if(strcmp((buf + 7),"keywait") == 0) {
				scrn_state = SCRN_KEYWAIT;
			}
		}
		if(strncmp(buf,"flags=",6) == 0) {
			for(x=6;x<strlen(buf);x++) {
				if(flag_setc(*(buf + x),1) == -1) {
					printf("%s '%c'\n","Unknown config option:",*(buf + x));
					close(file);
					exit(1);
				}
			}
		}
		if(strncmp(buf,"readbuf=",8) == 0) {
			buf_size = atoi(buf + 8);
		}
		if(strncmp(buf,"border=",7) == 0) {
			if(strncmp(buf+7,"off",3) == 0) {
				scrn_border = 0;
			}
			if(strncmp(buf+7,"on",2) == 0) {
				scrn_border = 1;
			}
		}
		for(i=0;i<CONF_SIZE;i++) {
			buf[0] = '\0';
		}
	}
	close(file);
	return 0;
}

int flag_setc(char c, int value) {

	switch(c) {
	case 'R':
		Rflag = value;
		return value;
		break;
	case 'v':
		vflag = value;
		return value;
		break;
	case 'H':
		Hflag = value;
		return value;
		break;
	case 'L':
		Lflag = value;
		return value;
		break;
	case 'P':
		Pflag = value;
		return value;
		break;
	case 'f':
		fflag = value;
		return value;
		break;
	case 'i':
		iflag = value;
		return value;
		break;
	case 'n':
		nflag = value;
		return value;
		break;
	case 'p':
		pflag = value;
		return value;
		break;
	case 't':
		tflag = value;
		return value;
		break;
	case 'd':
		dflag = value;
		return value;
		break;
	case 'I':
		Iflag = value;
		return value;
		break;
	case 'V':
		Vflag = value;
		return value;
		break;
	case 'm':
		mflag = value;
		return value;
		break;
	case 'h':
		hflag = value;
		return value;
		break;
	case 'u':
		uflag = value;
		return value;
		break;
	default:
		return -1;
		break;
	}
	return -1;
}
