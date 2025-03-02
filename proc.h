/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* note: sysprocs.h must be included first */

extern char	ShcomBuf[LBSIZE];

extern char	*MakeName(char *command);

extern void
	isprocbuf(const char *bufname),
	get_FL_info(char *, char *),
	ChkErrorLines(void),
	ErrFree(void);

extern wait_status_t
	UnixToBuf(int, const char *, const char *, const char *);

/* flags for UnixToBuf: */
#define UTB_DISP	1	/* Display output? */
#define UTB_CLOBBER	2	/* (if UTB_DISP)  clear buffer at start? */
#define UTB_ERRWIN	4	/* (if UTB_DISP) make window size error-window-size? */
#define UTB_SH	8	/* shell command? */
#define UTB_FILEARG	16	/* pass curbuf->b_fname as $0? */

#ifndef MSDOS_PROCS
extern pid_t	ChildPid;	/* pid of any outstanding non-iproc process */
extern void	dowait(wait_status_t *status);
#endif

/* Commands: */

#ifdef SUBSHELL
extern void
	MakeErrors(void),
	FilterRegion(void),
	ShNoBuf(void),
	ShToBuf(void),
	ShellCom(void),
	Shtypeout(void),
	ProcEnvExport(void),
	ProcEnvShow(void),
	ProcEnvUnset(void);
#endif
/*
 * Even if we don't have MakeErrors, the following are useful because we can
 * load an error file and parse it with these.
 */
extern void
	ErrParse(void),
	ShowErr(void),
	NextError(void),
	PrevError(void);

#ifdef SPELL
extern void
	SpelBuffer(void),
	SpelWords(void);
#endif

/* Variables: */

extern int	EWSize;			/* VAR: percentage of screen to make the error window */
extern char	ErrFmtStr[256];		/* VAR: format string for parse errors */
#ifdef SUBSHELL
extern jbool	WtOnMk;			/* VAR: write files on compile-it command */
extern jbool	WrapProcessLines;	/* VAR: wrap process lines at CO-1 chars */
#endif
#ifdef SPELL
extern char	SpellCmdFmt[FILESIZE];		/* VAR: command to use for spell check */
#endif
