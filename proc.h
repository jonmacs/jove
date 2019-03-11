/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/* note: sysprocs.h must be included first */

extern char	ShcomBuf[LBSIZE];

extern char	*MakeName proto((char *command));

extern void
	isprocbuf proto((char *bufname)),
	get_FL_info proto((char *, char *)),
	ChkErrorLines proto((void)),
	ErrFree proto((void));

extern wait_status_t	UnixToBuf proto((char *, char *, bool, int, bool, ...));

#ifndef	MSDOS
extern void	dowait proto((pid_t pid, wait_status_t *status));
#endif

/* Commands: */

#ifdef	SUBSHELL
extern void
	MakeErrors proto((void)),
	FilterRegion proto((void)),
	ShNoBuf proto((void)),
	ShToBuf proto((void)),
	ShellCom proto((void)),
	Shtypeout proto((void));
#endif
/*
 * Even if we don't have MakeErrors, the following are useful because we can
 * load an error file and parse it with these.
 */
extern void
	ErrParse proto((void)),
	ShowErr proto((void)),
	NextError proto((void)),
	PrevError proto((void));

#ifdef	SPELL
extern void
	SpelBuffer proto((void)),
	SpelWords proto((void));
#endif

/* Variables: */

extern int	EWSize;			/* size to make the error window */
extern char	ErrFmtStr[256];		/* format string for parse errors */
#ifdef	SUBSHELL
extern bool	WtOnMk;			/* write files on compile-it command */
#endif

/* sysdep.h:
 *	SigMask
 */
