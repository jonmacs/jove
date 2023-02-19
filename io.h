/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

extern char	*HomeDir;

extern size_t	HomeLen;

extern bool	DOLsave;	/* Do Lsave flag.  If lines aren't being saved
				   when you think they should have been, this
				   flag is probably not being set, or is being
				   cleared before lsave() was called. */

extern daddr	DFree;  /* pointer to end of tmp file */

extern int	Jr_Len;		/* length of Just Read Line */

extern long	io_chars;
extern int	io_lines;

extern char
	*lbptr proto((struct line *line)),
	*pr_name proto((char *fname,int okay_home)),
	*pwd proto((void));

extern struct FileStruct
	*open_file proto((char *fname,char *buf,int how,int complainifbad,int loudness));

extern void
	setCWD proto((char *d)),
	getCWD proto((void)),
	PathParse proto((char *name,char *intobuf)),
	SyncTmp proto((void)),
	close_file proto((struct FileStruct *fp)),
	d_cache_init proto((void)),
	file_write proto((char *fname, bool app)),
	jgetline proto((daddr addr,char *buf)),
	lsave proto((void)),
	putreg proto((struct FileStruct *fp,struct line *line1,int char1,struct line *line2,int char2,bool makesure)),
	read_file proto((char *file, bool is_insert)),
	put_bufs proto((bool askp)),
	tmpclose proto((void)),
	tmpremove proto((void));

extern bool
	chkCWD proto((char *dn));

extern daddr
	putline proto((char *buf));

/* Commands: */

extern void
	AppReg proto((void)),
	Chdir proto((void)),
	InsFile proto((void)),
	Popd proto((void)),
	Pushd proto((void)),
	ReadFile proto((void)),
	SaveFile proto((void)),
	WriteFile proto((void)),
	WtModBuf proto((void)),
	WrtReg proto((void)),
	prCWD proto((void)),
	prDIRS proto((void));

/* Variables: */

#ifdef	BACKUPFILES
extern bool	BkupOnWrite;		/* make backup files when writing */
#endif
extern int	CreatMode;		/* default mode for creat'ing files */
extern bool	EndWNewline;		/* end files with a blank line */
extern bool	OkayBadChars;		/* allow bad characters in files created by JOVE */
