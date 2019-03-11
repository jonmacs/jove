/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/*
 * System Feature Selection: describe OS and C system to JOVE.  See sysdep.doc
 * for a better description of what the different #define feature symbols
 * mean.  If you add new ones, please keep the syntax of the first #ifdef, so
 * that "grep System: sysdep.h" catches the first line of all symbols.
 */

#ifdef SUNOS5	/* System: Solaris 2.x, SunOS 5.x */
/* Currently identical to SYSVR4. */
# define TERMIOS	1
# define USE_GETCWD	1
# define FULL_UNISTD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define SVR4_PTYS	1
# define POSIX_PROCS	1
# define JOB_CONTROL	1
# define USE_UNAME	1
# define USE_GETPWNAM	1
# define USE_FSYNC	1
# define USE_FSTAT	1
# define USE_FCHMOD	1
#endif

#ifdef SUNOS41	/* System: SunOS4.1 to 4.1.3 */
/* main differences from SUNOS5 are BSD_PTYS, NO_STRERROR and SGTTY. */
# define SGTTY		1
# define USE_GETCWD	1
# define POSIX_UNISTD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define BSD_PTYS	1
# define POSIX_PROCS	1
# define BSD_SIGS	1
# define JOB_CONTROL	1
# define BSD_SETPGRP	1
# define USE_KILLPG	1
# define USE_GETPWNAM	1
# define USE_GETHOSTNAME	1
# define NO_STRERROR	1
# define USE_FSYNC	1
# define USE_FSTAT	1
# define USE_FCHMOD	1
# define USE_AVENRUN	1
#endif

#ifdef SUNOS40	/* System: SunOS4.0 to 4.0.3c */
/*
 * almost identical to SUNOS41, main difference is that SUNOS41 uses
 * POSIX_PROCS, this one uses BSD_WAIT/WAIT3, USE_MEMORY_H.
 */
# define SGTTY		1
# define USE_GETCWD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define BSD_PTYS	1
# define BSD_WAIT	1
# define WAIT3		1
# define BSD_SIGS	1
# define JOB_CONTROL	1
# define USE_VFORK	1
# define BSD_SETPGRP	1
# define USE_KILLPG	1
# define USE_GETPWNAM	1
# define USE_GETHOSTNAME	1
# define NO_STRERROR	1
# define USE_FSYNC	1
# define USE_FSTAT	1
# define USE_FCHMOD	1
# define USE_AVENRUN	1
# define USE_MEMORY_H	1
#endif

#ifdef SUNOS3	/* System: SunOS before 4.0, eg. 3.5. not recently tested */
/* This is very close to BSD4 */
# define SGTTY		1
# define USE_GETWD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define BSD_PTYS	1
# define BSD_WAIT	1
# define WAIT3		1
# define BSD_SIGS	1
# define JOB_CONTROL	1
# define USE_VFORK	1
# define BSD_SETPGRP	1
# define USE_KILLPG	1
# define BSD_DIR	1
# define USE_GETPWNAM	1
# define SIGRESTYPE	int
# define SIGRESVALUE	0
# define USE_GETHOSTNAME	1
# define NO_STRERROR	1
# define USE_FSYNC	1
# define USE_FSTAT	1
# define USE_FCHMOD	1
# define USE_AVENRUN	1
/* # define USE_BCOPY	1 */
/* # define USE_INDEX	1 */
#endif

#ifdef BSDPOSIX	/* System: Posix system with BSD flavouring for ptys */
/*
 * Use for BSD4.4, 386BSD, BSDI, NetBSD, OSF/1, even seems to work for
 * SunOS4.1.3!
 */
# define TERMIOS	1
# define USE_GETCWD	1
# define FULL_UNISTD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define BSD_PTYS	1
# define POSIX_PROCS	1
# define BSD_SIGS	1
# define JOB_CONTROL	1
# define BSD_SETPGRP	1
# define USE_KILLPG	1
# define USE_GETPWNAM	1
# define USE_GETHOSTNAME	1
# define USE_FSYNC	1
# define USE_FSTAT	1
# define USE_FCHMOD	1
# define USE_AVENRUN	1
#endif

#ifdef IRIX4	/* System: Irix4.0.1, should work back till 3.2 or 3.3 */
/*
 * This is a Posix system with its own way of doing PTYS.  Older versions may
 * need MIPS_CC_BUG defined as well.
 */
# define _BSD_COMPAT	1	/* Turn on BSD setpgrp and other neat things */
# define TERMIOS	1
# define USE_GETCWD	1
# define FULL_UNISTD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define IRIX_PTYS	1
# define POSIX_PROCS	1
# define BSD_SIGS	1
# define JOB_CONTROL	1
# define SIGRESTYPE	int
# define SIGRESVALUE	0
# define BSD_SETPGRP	1
# define USE_KILLPG	1
# define USE_GETHOSTNAME	1
#endif

#ifdef SYSVR4	/* System: System V, Release 4 and derivatives */
# define TERMIOS	1
# define USE_GETCWD	1
# define FULL_UNISTD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define SVR4_PTYS	1
# define POSIX_PROCS	1
# define JOB_CONTROL	1
# define USE_UNAME	1
# define USE_GETPWNAM	1
# define USE_FSYNC	1
# define USE_FSTAT	1
# define USE_FCHMOD	1
#endif

#ifdef HPUX	/* System: Hewlett-Packard HP-UX 9.01 */
# define TERMIOS	1
# define USE_GETCWD	1
# define POSIX_UNISTD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define BSD_PTYS	1
# define POSIX_PROCS	1
# define BSD_SIGS	1
# define JOB_CONTROL	1
# define USE_UNAME	1
# define DEFINE_PC_BC_UP_OSPEED	1	/* May be needed for all SYSVR2 */
#endif

#ifdef BSD4	/* System: Berkeley BSD4.x, 2.9, 2.10, MIPS RiscOS 4.x */
/*
 * MIPS needs -systype bsd43, older releases (before 4.50?) may need
 * MIPS_CC_BUG defined as well.
 */
# define SGTTY		1
# define USE_GETWD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define BSD_PTYS	1
# define BSD_WAIT	1
# define WAIT3		1
# define BSD_SIGS	1
# define JOB_CONTROL	1
# define USE_VFORK	1
# define BSD_SETPGRP	1
# define USE_KILLPG	1
# define BSD_DIR	1
# define SIGRESTYPE	int
# define SIGRESVALUE	0
# define USE_GETHOSTNAME	1
# define NO_STRERROR	1
# define USE_FSYNC	1
# define USE_FSTAT	1
# define USE_FCHMOD	1
# define USE_AVENRUN	1
# define USE_BCOPY	1
# define USE_INDEX	1
#endif



#ifdef ZORTECH	/* System: Zortech C V3.0 for the IBM-PC under MSDOS */
# define MSDOS		1
# define IBMPC		1
# define getch		jgetch	/* UGH!  Zortech steals from our namespace. */
# define MALLOC_CACHE	1	/* DGROUP gets full otherwise */
# define NBUF		62	/* NBUF*JBUFSIZ must be less than 64 kB */
# define REALSTDC	1	/* close enough for us, but ZORTECH doesn't define __STDC__ */
#endif

/*
 * All the systems marked with XXX_ are ones that this version of Jove (4.15)
 * has not been tested on.  4.15 was the transition from implicit #ifdefs
 * scattered throughout the code to feature-based ifdefs that MUST be
 * enabled in sysdep.h.  The #ifdef XXX_* below have been left in to provide a
 * guide to re-porting this version to those architectures.  If you do so
 * successfully, please send a copy of these changes to
 * jovehacks@cs.toronto.edu and we'll try to incorporate those changes above
 * and get rid of the XXX_.
 */

#ifdef XXX_M_XENIX	/* System: Microsoft or SCO Xenix */
/* #define NBUF	48 */	/* if we are on a 286, NBUF*JBUFSIZ must be less than 64 kB */
# define BSD_DIR	1
#endif

#ifdef XXX_SYSV		/* System: System V Rel. 2, System III */
# define TERMIO		1
# define USE_PWD
# define NONBLOCKINGREAD	1
# define USE_MEMORY_H	1
# define DIRENT_EMULATE	1	/* for truly old versions? */
#endif

#ifdef XXX_A_UX		/* System: A/UX on a MacII (Do *not* define "MAC") */
/* It might be better to try BSDPOSIX for newer A/UX. */
# define BSD_WAIT	1
# define BSD_DIR	1
# define WAIT3		1
# define BSD_SIGS	1
# define USE_KILLPG	1
# define TERMIO		1	/* uses termio struct for terminal modes */
# define USE_GETHOSTNAME	1
# define USE_SELECT	1
#endif

#ifdef XXX_AIX		/* System: IBM AIX */
/* from guttman@mashie.ece.jhu.edu via buchanan@cs.ubc.ca */
# define BSD_DIR
# define USE_GETWD
# define TERMIO	1	/* uses termio struct for terminal modes */
# define USE_UNAME
# define USE_SELECT	1
# define USE_SELECT_H	1
#endif
  
#ifdef XXX_OLDMIPS	/* System: MIPS-SYSV, Irix before 3.3. */
/*
 * Older MIPS (UMIPS-SYSV, anything other than their 4.3 port before
 * RISCOS4.x) and SGI 4D OSes (anything before Irix3.3) have BSD style wait,
 * and directory routines if you link -lbsd and define -I/usr/include/bsd on
 * the compile line. But they have SysV style signals.  Jove was ported to the
 * SGI 68K boxes once, but it the mods seem to have been lost.
 */
# define BSD_WAIT	1	/* Berkeley style sys/wait.h */
# define BSD_DIR	1	/* Berkeley style dirent routines */
#endif

#ifdef XXX_MSC51	/* System: Microsoft C 5.1 on IBM PC under DOS*/
/* This hasn't been tested recently.  Consider stealing ZORTECH settings. */
# define MSDOS	1
# define IBMPC	1
# define NO_PTRPROTO 1
# define REALSTDC	1	/* well, almost */
# ifdef	M_I86LM		/* not large memory model */
#  define NBUF		62	/* NBUF*JBUFSIZ must be less than 64 kB */
# else
#  define SMALL		1
# endif
#endif

#ifdef	XXX_THINK_C	/* System: an old version of Think C on the Macintosh */
# define MAC 1
/* Think C does not have a "defined" preprocessor operator.
 * This kludge is intended to avoid the problem.
 * ??? Perhaps Think C has been fixed by now. -- DHR
 */
# define defined(x) (x)
  typedef int size_t;	/* Not unsigned!?! */
# define byte_zero(s,n) setmem((s),(n),0)
# define NO_PTRPROTO	1
# define MALLOC_CACHE	1   /* Only 32K of static space on Mac, so... */
#endif


/*************************************************************************/
/*
 * The things below here aren't meant to be tuned, but are included here
 * because they're dependent on the things defined earlier in the file.
 */
#ifdef USE_BCOPY
# define byte_copy(from, to, len)	bcopy((UnivConstPtr)(from), (UnivPtr)(to), (size_t)(len))
# define byte_zero(s, n)	bzero((UnivPtr)(s), (size_t)(n))
#endif

#ifndef	byte_copy
# ifdef	USE_MEMORY_H
#  include <memory.h>
# endif
# define byte_copy(from, to, count)	memcpy((UnivPtr)(to), (UnivConstPtr)(from), (size_t)(count))
# define byte_zero(s, n)		memset((UnivPtr)(s), 0, (size_t)(n))
#endif

#ifdef USE_INDEX
# define strchr		index
# define strrchr	rindex
#endif

#ifdef FULL_UNISTD
# define POSIX_UNISTD	1
#endif

/* Determine if really ANSI C */
#ifdef	__STDC__
# if	__STDC__ >= 0
#  define REALSTDC 1
# endif
#endif

/*
 * The operating system (MSDOS or MAC) must be defined by this point.  IBMPC
 * is defined in the Makefile.
 */
#ifndef	MSDOS
# ifndef	MAC
#   define UNIX	1	/* default to UNIX */
# endif
#endif

#ifdef	pdp11
# define SMALL	1
#endif

#ifdef SMALL
# define NBUF		3
# define JLGBUFSIZ	9
#endif

#ifndef NBUF
# define NBUF 64
#endif

#ifndef	JLGBUFSIZ
# define JLGBUFSIZ 10
#endif

/* Maximum length of a buffer and hence a line, among other things. */
#define	JBUFSIZ	(1<<JLGBUFSIZ)

/* Our defaults tend to be conservative and lean towards pure SYSV */
#ifdef	UNIX
# define TERMCAP	1
# define ASCII7		1
# ifndef PTYPROCS
#  define PIPEPROCS	1	/* use pipes */
# endif
# if !defined(TERMIOS) && !defined(SGTTY)
#  define TERMIO	1	/* uses termio struct for terminal modes */
# endif
/*
 * At the moment, the PTY code mandates having select().  One day, this might
 * change.
 */
# if defined(PTYPROCS) && !defined(USE_SELECT)
   sysdep.h: Sorry, PTYPROCS requires the select() system call.  You must
   either define USE_SELECT or undefine PTYPROCS.
# endif
# if defined(SIGCLD) && !defined(SIGCHLD)
#  define SIGCHLD	SIGCLD
# endif
#endif /* UNIX */

#ifdef	ASCII7	/* seven bit characters */
# define NCHARS 0200
#else
# define NCHARS 0400
#endif

#define CHARMASK (NCHARS - 1)

#ifndef	MSDOS
# define FILESIZE	256
#else	/* MSDOS */
# define FILESIZE	64
#endif	/* MSDOS */

#ifndef	SIGRESTYPE	/* default to void, correct for most modern systems */
# define SIGRESTYPE	void
# define SIGRESVALUE	/*void!*/
#endif

#ifdef	UNIX
# ifdef	BSD_SIGS
extern long	SigMask;

#  define SigHold(s)	sigblock(SigMask |= sigmask((s)))
#  define SigRelse(s)	sigsetmask(SigMask &= ~sigmask((s)))
# else
#  define signal	sigset
#  define SigHold(s)	sighold(s)
#  define SigRelse(s)	sigrelse(s)
# endif
#endif

/* On a system which limits JOVE to a very small data segment,
 * it may be worthwhile limiting daddr to a short.  This reduces
 * the size of a Line descriptor, but reduces the addressable size
 * of the temp file.  This is reasonable on a PDP-11 and perhaps
 * an iAPX*86.
 */

#ifdef	SMALL
  typedef unsigned short	daddr;
#else
  typedef unsigned long	daddr;
#endif	/* SMALL */

#define	NULL_DADDR		((daddr) 0)
