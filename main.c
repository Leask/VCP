/* 
 * Copyright (c) 2003,2004  Daniel Bryan
 * All rights reserved.
 *
 * For more information see COPYRIGHT.
 */

#include <unistd.h>
#include <fts.h>

#include "common.h"
#include "screen.h"
#include "misc.h"

struct dest_new dest;

int use_color = 0;
int use_curses = 0;
int ret = 0;
int scrn_state = SCRN_KEYWAIT;
int scrn_border = 0;

int buf_size = 0;
int Rflag,vflag,Hflag,Pflag,Lflag,fflag,iflag,\
	tflag,pflag,dflag,hflag,mflag,Vflag,Iflag,\
	nflag,uflag;

unsigned long totalfiles;
unsigned long curfile;
unsigned long goodcp;

/* used to read/write file data */
char *databuf = NULL;

int main(int argc, char *argv[]) {	
	
	struct stat st,st2;
	int fts_opt;
	int ch;
	int exist = 0;
	char *targv = NULL;
	int i = 0;
	
	fts_opt = FTS_NOCHDIR | FTS_PHYSICAL;
	
	conf_read(1); /* global */
	conf_read(0); /*  user  */

	while ((ch = getopt(argc, argv, "HLPRfinpvtdIVmhb:u")) != -1) {
		if(ch == 'b') {
			buf_size = atoi(optarg);
		}
		else if(flag_setc(ch,1) == -1) {
			usage();
		}
	}
	
	argc -= optind;
	argv += optind;
	
	if(Vflag) {
		printf("VCP %d.%d\n",VER_MAJ,VER_MIN);
		return 0;
	}
	
	if(hflag) {
		printf("VCP %d.%d\n",VER_MAJ,VER_MIN);
		printf("usage:\n");
		printf("vcp [options] src dest\n");
		printf("vcp [options] src1 src2 .. destdir\n");
		printf("vcp [options -m] src dest1 dest2 ..\n");
		printf("\n");
		printf("options:\n");
		printf("-R\tcopies directories, re-creates special files\n");
		printf("-v\tverbose output\n");
		printf("-H\tfollows symlinks on cmd line (needs -R)\n");
		printf("-L\tall symlinks are followed (needs -R)\n");
		printf("-P\tno symlinks are followed (needs -R, default)\n");
		printf("-f\tforces existing files to be overwritten\n");
		printf("-i\tprompts when existing file found (Y/[N])\n");
		printf("-I\tprompts when existing file found ([Y]/N)\n");
		printf("-n\tno existing files are overwritten\n");
		printf("-p\tattempts to keep all permissions\n");
		printf("-t\toutput to console, use with -v for status\n");
		printf("-d\tdelayed copy\n");
		printf("-V\tprint version and exit\n");
		printf("-m\tmulti-output file copy\n");
		printf("-b BUF\tset read buffer size (bytes)\n");
		printf("-u\tfiles present with newer time stamp and same size are not copied\n");
		printf("-h\tprint this\n");
		return 0;
	}

	if(argc < 2)
		usage();

	if(nflag && iflag) {
		printf("-i and -n conflict\n");
		usage();
	}
	if(fflag && nflag) {
		printf("-f and -n conflict\n");
		usage();
	}
	if(fflag && iflag) {
		printf("-f and -i conflict\n");
		usage();
	}

	scrn_init();
	
	if(Rflag && !Pflag && !Hflag && !Lflag)
		Pflag = 1; /* default for -R */

	if(Hflag && !Rflag) {
		Hflag = 0;
		logadds(LOG_VRB,"-H ignored",NULL,NULL);
	}
	if(Pflag && !Rflag) { 
		Pflag = 0;
		logadds(LOG_VRB,"-P ignored",NULL,NULL);
	}
	if(Lflag && !Rflag) {
		Lflag = 0;
		logadds(LOG_VRB,"-L ignored",NULL,NULL);
	}
	
	if(Pflag) {
		if(Lflag) {
			Lflag = 0;
			logadds(LOG_VRB,"-L ignored",NULL,NULL);
		} 
		if(Hflag) {
			Hflag = 0;
			logadds(LOG_VRB,"-H ignored",NULL,NULL);
		} 
	}
	if(Hflag && Lflag) {
		Lflag = 0;
		logadds(LOG_VRB,"-L ignored",NULL,NULL);
	}
	
	if(Iflag && iflag) {
		Iflag = 0;
		logadds(LOG_VRB,"-I ignored",NULL,NULL);
	}
	if(Iflag)
		iflag = 1;
	
	if(Rflag) {
		if(Hflag)
			fts_opt |= FTS_COMFOLLOW;
		if(Lflag) {
			fts_opt &= ~FTS_PHYSICAL;
			fts_opt |= FTS_LOGICAL;
		}
	} else {
		fts_opt &= ~FTS_PHYSICAL;
		fts_opt |= FTS_LOGICAL;
	}
	
	scrn_upd_file(NULL,NULL);
	scrn_upd_part(0,0);
	
	if(vflag)
		flags_print();
	
	if(mflag) {
		
		if(stat(argv[0],&st) == -1) {
			endwin();
			use_curses = 0;
			logadds(LOG_ERR,"%s: Not a vaild source",argv[0],NULL);
			usage();
		}

		targv = argv[1];
		argv[1] = NULL;
		totalfiles = cntfiles(argv,fts_opt) * (argc - 1);
		argv[1] = targv;
		
		if(S_ISDIR(st.st_mode) && !Rflag) {
			endwin();
			use_curses = 0;
			logadds(LOG_ERR,"%s is a directory, not copied",argv[0],NULL);
			ret = 1;
			done(argc-1);
		}
		
		for (i=1;i<argc;i++){
			dest.opath = argv[i];
			argv[1] = NULL;
			if(S_ISDIR(st.st_mode))
				copyall(argv,fts_opt,T_DIR);
			else
				copyall(argv,fts_opt,T_FILE);
			argv[1] = targv;
		}
		done(argc-1);
		
	}
	
	
	dest.opath = argv[argc-1];
	argv[argc-1] = NULL;
	totalfiles = cntfiles(argv,fts_opt);	
	
	if(stat(dest.opath,&st) == -1) {
		if(argc != 2) {
			endwin();
			use_curses = 0;
			logadds(LOG_ERR,"%s: Not a vaild destination",dest.opath,NULL);
			usage();
		}
		exist = 0;
	} else
		exist = 1;
	
	if(!exist || !S_ISDIR(st.st_mode)) {
		if(argc != 2) {
			endwin();
			use_curses = 0;
			logadds(LOG_ERR,"%s: Not a vaild destination",dest.opath,NULL);
			usage();
		}
		statit(argv[0],&st2,1);
		if(!exist) {
			if(S_ISDIR(st2.st_mode)) {
				copyall(argv,fts_opt,T_NED);  /* DIR  -> !DIR */
			} else {
				copyall(argv,fts_opt,T_FILE); /* FILE -> FILE */
			}
		} else {
			copyall(argv,fts_opt,T_FILE);     /* FILE -> FILE */
		}
		done(0);
	} else {
		if(S_ISDIR(st.st_mode)) {
			check_iloop(argv,dest.opath); /* check for loop */
			copyall(argv,fts_opt,T_DIR);	  /*      ->  DIR */
			done(0);
		}
		
		endwin();
		use_curses = 0;		
		logadds(LOG_ERR,"%s: Not a valid destination",dest.opath,NULL);
		usage();
	}
	return 1;
}
