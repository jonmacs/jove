/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

extern char
	*lbptr proto((struct line *line)),
	*pr_name proto((char *fname,int okay_home)),
	*sprint proto((char *, ...));

extern struct file
	*open_file proto((char *fname,char *buf,int how,int ifbad,int loudness));

extern void
	PathParse proto((char *name,char *intobuf)),
	SyncTmp proto((void)),
	close_file proto((struct File *fp)),
	d_cache_init proto((void)),
	file_write proto((char *fname,int app)),
	filemunge proto((char *newname)),
	getline proto((daddr addr,char *buf)),
	lsave proto((void)),
	putreg proto((struct File *fp,struct line *line1,int char1,struct line *line2,int char2,int makesure)),
	read_file proto((char *file,int is_insert)),
	tmpclose proto((void)),
	tmpinit proto((void));

extern daddr
	f_getputl proto((struct line *line,struct File *fp)),
	putline proto((char *buf));
