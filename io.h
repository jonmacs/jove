/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern const char	*HomeDir;

extern size_t	HomeLen;

extern jbool	DOLsave;	/* Do Lsave flag.  If lines aren't being saved
				   when you think they should have been, this
				   flag is probably not being set, or is being
				   cleared before lsave() was called. */

extern daddr	DFree;	/* pointer to end of tmp file */

extern int	Jr_Len;		/* length of Just Read Line */

extern long
		io_chars,
		io_lines;

extern int	MakeTemp(char *, const char *);

extern char
	*lbptr(LinePtr),
	*pr_name(const char *, jbool),
	*pwd(void);

extern File
	*open_file(char *, char *, int, jbool);

extern void
	setCWD(char *),
	getCWD(void),
	PathCat(char *, size_t, const char *, const char *),
	PathParse(const char *, char *),
	SyncTmp(void),
	close_file(File *),
	d_cache_init(void),
	file_write(char *, jbool),
	jgetline(daddr, char *),
	lsave(void),
	putreg(File *, LinePtr, int, LinePtr, int, jbool),
	read_file(char *, jbool),
	put_bufs(jbool),
	tmpclose(void),
	tmpremove(void);

extern jbool
	chkCWD(char *);

extern daddr
	jputline(char *);

/* Commands: */

extern void
	AppReg(void),
	Chdir(void),
	InsFile(void),
	Popd(void),
	Pushd(void),
	Pushlibd(void),
	JReadFile(void),
	SaveFile(void),
	JWriteFile(void),
	WtModBuf(void),
	WrtReg(void),
	prCWD(void),
	prDIRS(void),
	backup_name(const char *, const char *, char *, size_t);

/* Variables: */

#ifdef BACKUPFILES
extern jbool	BkupOnWrite;		/* VAR: make backup files when writing */
#endif
#ifdef UNIX
extern int	CreatMode;		/* VAR: default mode for creat'ing files */
#endif
#ifdef MAC
# define CreatMode	0	/* dummy */
#endif
extern jbool	EndWNewline;		/* VAR: end files with a blank line */
extern jbool	OkayBadChars;		/* VAR: allow bad characters in files created by JOVE */
