/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/* Tuning: configure JOVE to installer's taste */

#define TUNED		1	/* don't touch this */

#include "sysdep.h"

/*
 * Select or comment out as many of the following features of Jove as you'd
 * like.  In general, most of them are useful or pleasant, but many of them
 * are non-portable, or make Jove larger.
 *
 * By default, most are included if appropriate.  BAREBONES suppresses most
 * of them.  With BAREBONES defined, Jove should compile on a lot of systems.
 * It should be considered the starting point for any port of Jove to a new
 * system.  An advantage of BAREBONES is that it shrinks JOVE's already
 * modest size.
 */

#ifndef BAREBONES

#define ABBREV		1	/* word abbreviation mode */
#define BACKUPFILES	1	/* enable the backup files code */

#ifdef	UNIX
#define BIFF		1	/* if you have biff (or the equivalent) */
#endif

#define CMT_FMT		1	/* include the comment formatting routines */
#define F_COMPLETION	1	/* filename completion */

/* Include code to IDchar (optimize for terminals with character
 * insert/delete modes
 */
#ifdef	TERMCAP
#define ID_CHAR		1	
#endif

#ifdef	UNIX
#define IPROCS		1	/* interactive processes */
#endif

/* Use the load average for various commands.
 * Do not define LOAD_AV if you lack a load average
 * system call and kmem is read protected.
 */
#ifdef	UNIX
/* # define LOAD_AV	1 */
#endif

#define LISP		1	/* include the code for Lisp Mode */
#define SPELL		1	/* spell words and buffer commands */
#define	SUBSHELL	1	/* enable various uses of subshells */

#endif /* !BAREBONES */

#define DFLT_MODE	0666	/* file will be created with this mode */

/* If the compiler does not support void, use -Dvoid=int or
 * typedef int	void;
 */

/*
 * USE_PROTOTYPE must be defined for compilers that support prototypes but are
 * NOT ANSI C, i.e. do not have __STDC__ == 1.  USE_PROTOTYPE will be
 * automatically defined for ANSI compilers.
 */
/* #define USE_PROTOTYPES	1 */

/* These are here since they define things in tune.c.  If you add things to
   tune.c, add them here too, if necessary. */

extern char
	*d_tempfile,
	*p_tempfile,
#ifdef	ABBREV
	*a_tempfile,
#endif
	*Recover,
	*Joverc,

#if	defined(IPROCS) && defined(PIPEPROCS)
	*Portsrv,
#endif

#ifdef	MSDOS
	CmdDb[],	/* path for cmds.doc */
#else
	*CmdDb,	/* path for cmds.doc */
#endif

	TmpFilePath[FILESIZE];	/* directory/device to store tmp files */

#ifdef	SUBSHELL
extern char
	Shell[FILESIZE],		/* shell to use */
	ShFlags[16];
#endif
