/* 
 * Copyright (c) 2003,2004  Daniel Bryan
 * All rights reserved.
 *
 * For more information see COPYRIGHT.
 */

#include <err.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "path.h"
#include "misc.h"
#include "copy.h"
#include "screen.h"

#ifndef _POSIX_SOURCE
#define st_mtime st_mtimespec.tv_sec
#endif

int copyall(char *args[],int fts_opt,int type) {
	FTS *ftsarg;
	FTSENT *ftsf;
	struct stat st;
	int dne = 0;
	int ret = 0;
	mode_t mode,mask;
	int base = 0;
		
	mask = ~umask(0777);
	umask(~mask);
		
	if(pathchsize(dest.opath,dest.opath)) {
		ret = 1;
		return 1;
	}
	
	ftsarg = fts_open(args,fts_opt,0);
	if(ftsarg == NULL) {
		logadds(LOG_ERR,"fts: %s",strerror(errno),NULL);
		ret = 1;
		return 1;
	}
	
	while((ftsf = fts_read(ftsarg)) != NULL) {
		
		switch(ftsf->fts_info) {
		case FTS_NS:
		case FTS_DNR:
		case FTS_ERR:
			print_error(ftsf->fts_path,ftsf->fts_errno);
			/* cntfiles() does not count FTS_DNR */
			if(ftsf->fts_info != FTS_DNR)
				curfile++;
			ret = 1;
			continue;
		case FTS_DC:
			logadds(LOG_ERR,"%s: directory causes a cycle",ftsf->fts_path,NULL);
			ret = 1;
			continue;
		}

		dest.path[0] = '\0';
		strcpy(dest.path,dest.opath);
			
		if(type != T_FILE) {
			if(type == T_DIR) {
				if(ftsf->fts_level == FTS_ROOTLEVEL) {
					base = pathnamed(ftsf->fts_path);
				}
			} else if(type == T_NED) {
				if(ftsf->fts_level == FTS_ROOTLEVEL)
					base = ftsf->fts_pathlen;
			}
			if(pathdadd(dest.path,ftsf->fts_path+base))
				continue;
		}

		if(ftsf->fts_info == FTS_DP) {
			if(!ftsf->fts_number) {
				continue;
			}
			if(setperm(dest.path,ftsf->fts_statp) != 0) {
				mode = ftsf->fts_statp->st_mode;
				if((mode & (S_ISUID | S_ISGID)) || \
				  ((mode | S_IRWXU) & mask) != (mode & mask)) {
				  	if(chmod(dest.path,mode & mask) != 0) {
				  		logadds(LOG_ERR,"chmod: %s",strerror(errno),NULL);
				  	}
				}
			}
			continue;
		}
		
		if(stat(dest.path,&st) == -1) {
			dne = 1; /* file doesn't exist */
		} else {
			if((st.st_dev==ftsf->fts_statp->st_dev) &&
			   (st.st_ino==ftsf->fts_statp->st_ino)) {
				logadds(LOG_ERR,"%s and %s are identical, not copied",\
					ftsf->fts_path,dest.path);
				if(S_ISDIR(st.st_mode))
					fts_set(ftsarg,ftsf,FTS_SKIP);
				continue;
			}
			if(!S_ISDIR(ftsf->fts_statp->st_mode)&&(S_ISDIR(st.st_mode))){
				logadds(LOG_ERR,"Can not overwrite %s with non-directory %s",\
					dest.path,ftsf->fts_path);
				ret = 1;
				continue;
			}
			
			if(!S_ISDIR(ftsf->fts_statp->st_mode) && uflag) {
				if(st.st_size == ftsf->fts_statp->st_size &&\
					difftime(st.st_mtime,ftsf->fts_statp->st_mtime) > 0) {
					
					logadds(LOG_VRB,\
						"%s: File is present with newer time stamp and same size, not copied",\
						dest.path,NULL);
					continue;
				}
				
			}
			dne = 0;
		}
			
		switch(ftsf->fts_statp->st_mode & S_IFMT) {
		/* link */
		case S_IFLNK:
			curfile++;
			copylink(ftsf->fts_path,dest.path) ? ret = 1 : 0;
			break;
		/* directory */
		case S_IFDIR:
			if(!Rflag) {
				logadds(LOG_ERR,"%s is a directory, not copied",ftsf->fts_path,NULL);
				fts_set(ftsarg,ftsf,FTS_SKIP);
			} else {
				if(dne) {
					if(mkdir(dest.path,\
						ftsf->fts_statp->st_mode | S_IRWXU) < 0) {
						logadds(LOG_ERR,"%s: Can not create directory",\
							dest.path,NULL);
						fts_set(ftsarg,ftsf,FTS_SKIP);
					} else
						logadds(LOG_VRB,"%s: Created",dest.path,NULL);
				} else if (!S_ISDIR(st.st_mode)) {
					logadds(LOG_ERR,\
						"Can not overwrite non-directory %s with %s",\
						dest.path,ftsf->fts_path);
					fts_set(ftsarg,ftsf,FTS_SKIP);
				}
				ftsf->fts_number = (pflag) || dne;
			}
			break;
		/* special */
		case S_IFBLK:
		case S_IFCHR:
			curfile++;
			if(Rflag)
				copyspecial(dest.path,ftsf->fts_statp) ? ret = 1 : 0;
			else
				copyfile(ftsf->fts_path,dest.path,ftsf->fts_statp) ? ret = 1 : 0;
			break;
		/* fifo */
		case S_IFIFO:
			curfile++;
			if(Rflag)
				copyfifo(dest.path,ftsf->fts_statp) ? ret = 1 : 0;
			else
				copyfile(ftsf->fts_path,dest.path,ftsf->fts_statp) ? ret = 1 : 0;
			break;
		/* file or other */
		default:
			curfile++;
			copyfile(ftsf->fts_path,dest.path,ftsf->fts_statp) ? ret = 1 : 0;
			break;
		}
	}
	if(errno) {
		logadds(LOG_ERR,"%s",strerror(ftsf->fts_errno),NULL);
		ret = 1;
		return 1;
	}
	return 0;
}
