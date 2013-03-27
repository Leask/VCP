/* 
 * Copyright (c) 2003,2004  Daniel Bryan
 * All rights reserved.
 *
 * For more information see COPYRIGHT.
 */
 
#define LOG_ERR -1 /* LOG ERROR */
#define LOG_MSG  0 /* LOG MESSAGE */
#define LOG_VRB  1 /* LOG VERBOSE */

int logget(char *name);
int logaddi(int code, char *base, int var, int var2);
int logadds(int code, char *base, char *var, char *var2);
