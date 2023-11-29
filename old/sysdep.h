/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/*
 * All the systems in this file have not been tested in the
 * current version of Jove.  The ones with XXX_ were not even
 * tested in the last version (Jove 4.16, circa 1996 CE).
 * The XXX* symbos are preserved have been left in to
 * provide a guide to re-porting this version to those
 * architectures.  If you do so successfully, please send a
 * copy of these changes to github.com/jonmacs/jove and we'll
 * try to incorporate those changes above and get rid of the
 * XXX_, and move the tested definition back to sysdep.h.

#ifdef SUNOS41	/* System: SunOS4.1 to 4.1.3 */
# define TERMIOS	1
# define NO_IOCTL_H_TTY	1	/* <sys/ioctl.h> conflicts with <termios.h>? */
# define USE_GETCWD	1
# define POSIX_UNISTD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define BSD_PTYS	1	/* beware security flaw! */
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
# define HAS_SYMLINKS	1
# define USE_CTYPE	1
#endif

#ifdef SUNOS40	/* System: SunOS4.0 to 4.0.3c */
/* Almost identical to SUNOS41, main difference is that SUNOS41 uses
 * POSIX_PROCS, this one uses BSD_WAIT/WAIT3, USE_MEMORY_H.
 * Beware: TERMIO under SunOS 4.0 does not allow VSUSP to be changed,
 * so we cannot use it.
 */
/* try to patch over <termios.h> problems */
# define TERMIOS	1
# define setsid()	/* SunOS 4.0 apparently has no concept of session */
# define cfgetospeed(p)	(CBAUD & (p)->c_cflag)
# define tcgetattr(fd, p)	ioctl((fd), TCGETS, (UnivPtr)(p))
# define tcsetattr(fd, oa, p)	ioctl((fd), (oa), (UnivPtr)(p))
/* fake values for "optional_actions" (oa) arg to tcsetattr */
#  define TCSANOW	TCSETS
#  define TCSADRAIN	TCSETSW
#  define TCSAFLUSH	TCSETSF
/* end of <termios.h> patches */

# define NO_IOCTL_H_TTY	1	/* <sys/ioctl.h> conflicts with <termios.h>? */
# define USE_GETCWD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define BSD_PTYS	1	/* beware security flaw! */
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
# define HAS_SYMLINKS	1
# define USE_MEMORY_H	1
#endif

#ifdef SUNOS3	/* System: SunOS before 4.0, eg. 3.5. not recently tested */
/* This is very close to BSD4 */
# define SGTTY		1
# define USE_GETWD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define BSD_PTYS	1	/* beware security flaw! */
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
# define HAS_SYMLINKS	1
/* # define USE_BCOPY	1 */
/* # define USE_INDEX	1 */
#endif

#ifdef AIX3_2	/* System: IBM RS6000 running AIX 3.2 */
# define AIX		1
# define FULL_UNISTD	1
# define USE_GETWD	1
# define TERMIOS	1	/* uses termio struct for terminal modes */
# define USE_UNAME	1
# define USE_SELECT	1
# define USE_SELECT_H	1
# define PTYPROCS	1
# define BSD_PTYS	1	/* beware security flaw! */
# define NO_EOF_FROM_PTY    1	/* BUG! */
# define POSIX_PROCS	1
# define WAIT3		1
# define POSIX_SIGS	1
# define JOB_CONTROL	1
# define USE_GETPWNAM	1
# define USE_UNAME	1
# define USE_FSYNC	1
# define USE_FSTAT	1
# define USE_FCHMOD	1
# define HAS_SYMLINKS	1
# define USE_CTYPE	1
#endif

#ifdef AIX4_2	/* System: IBM AIX 4.2 */
# define AIX		1
# define FULL_UNISTD	1
# define USE_GETWD	1
# define TERMIOS	1	/* uses termio struct for terminal modes */
# define USE_UNAME	1
# define USE_SELECT	1
# define USE_SELECT_H	1
# define PTYPROCS	1
# define BSD_PTYS	1	/* beware security flaw! */
# define NO_EOF_FROM_PTY    1	/* BUG! */
# define POSIX_PROCS	1
# define WAIT3		1
# define POSIX_SIGS	1
# define JOB_CONTROL	1
# define USE_GETPWNAM	1
# define USE_UNAME	1
# define USE_FSYNC	1
# define USE_FSTAT	1
# define USE_FCHMOD	1
# define HAS_SYMLINKS	1
# define USE_CTYPE	1
  /* Only difference from AIX3_2, perhaps due to switching from
   * -ltermcap to -lcurses (-ltermcap seems to have disappeared):
   */
# define DEFINE_PC_BC_UP_OSPEED	1	/* May be needed for all SYSVR2 */
#endif

#ifdef _convex__	/* System: ConvexOS (versions 10.x - 11.x) */
/* Note: this must be placed before BSDPOSIX ifdef (we define BSDPOSIX). */
# define BSDPOSIX	1	/* mostly like BSDPOSIX */
# define STICKY_TTYSTATE	1	/* fudge: many progs stupidly set ISTRIP */
# define ISO_8859_1	1	/* fudge: <ctype.h> doesn't work for 8-bit chars, but X does */
#endif

#ifdef _QNX__	/* System: QNX OS for x86 family */
/* Note: this must be placed before BSDPOSIX ifdef (we define BSDPOSIX). */
# define BSDPOSIX	1
# define ONLCR		OPOST	/* how to do ONLCR */
# define USE_SELECT_H	1
#endif

#ifdef IRIX
# define _BSD_COMPAT	1	/* Turn on BSD setpgrp and other neat things */
# define TERMIOS	1
# define USE_GETCWD	1
# define FULL_UNISTD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define POSIX_PROCS	1
# define POSIX_SIGS	1
# define JOB_CONTROL	1
# ifdef IRIX4
   /* Should work for IRIX 4.0.4 back to 3.2 or 3.3.  This is a Posix system
    * with its own way of doing PTYS.  Older versions may need MIPS_CC_BUG
    * defined as well.
    */
#  define IRIX_PTYS	1
#  define NO_TIOCREMOTE	1
#  define SIGRESTYPE	int
#  define SIGRESVALUE	0
# else
   /* IRIX 5 and later */
#  define SVR4_PTYS	1
#  define NO_TIOCREMOTE	1
#  define NO_TIOCSIGNAL	1
#  define SIGRESTYPE	void
#  define SIGRESVALUE	/*void!*/
# endif
# define BSD_SETPGRP	1
# define USE_GETPWNAM	1
# define USE_KILLPG	1
# define USE_GETHOSTNAME	1
# define HAS_SYMLINKS	1
# define USE_CTYPE	1
#endif

#ifdef HPUX	/* System: Hewlett-Packard HP-UX 9.01 & 11, probably others */
# define TERMIOS	1
# define USE_BSDTTYINCLUDE	1	/* No other way to turn off ^Y */
# define USE_GETCWD	1
# define FULL_UNISTD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define BSD_PTYS	1	/* beware security flaw! */
# define POSIX_PROCS	1
# define NO_EOF_FROM_PTY    1	/* BUG! */
# define POSIX_SIGS	1
# define JOB_CONTROL	1
# define USE_UNAME	1
# define DEFINE_PC_BC_UP_OSPEED	1	/* May be needed for all SYSVR2 */
# define HAS_SYMLINKS	1
#endif

#ifdef SCO_ODT3	/* System: SCO ODT 3.0 */
# define TERMIOS	1
/* # define FULL_UNISTD	1 */	/* Not tested!  May be worth trying. */
# define USE_GETCWD	1
# define POSIX_UNISTD	1
# define USE_SELECT	1
# define PTYPROCS	1
# define BSD_PTYS	1	/* beware security flaw! */
# define POSIX_PROCS	1
# define JOB_CONTROL	1
# define USE_UNAME	1
/* In SCO ODT 3.0, a wait() will never finish if SIGCHLD is being held.
 * We think that this is a bug.  It's rumoured to be "fixed" in the next
 * release.  JOVE's IPROCS code no longer triggers this bug.
 */
# define PTYPROCS	1
# define HAS_SYMLINKS	1
# define USE_CTYPE	1
#endif

#ifdef ZTCDOS	/* System: Zortech C V3.0 for the IBM-PC under MSDOS */
# define IBMPCDOS		1
# define getch		jgetch	/* UGH!  Zortech steals from our namespace. */
# define MALLOC_CACHE	1	/* DGROUP gets full otherwise */
# define REALSTDC	1	/* close enough for us, but ZTCDOS doesn't define __STDC__ */
# ifdef M_I86LM		/* large memory model */
#  define NBUF		62	/* NBUF*JBUFSIZ must be less than 64 kB */
# else
#  define NBUF		3
#  define FAR_LINES	1	/* to squeeze larger files, distance Lines */
# endif
  /* (1) specify stack size, and
   * (2) request support of wildcards in command-line args (UGH!)
   */
# define STACK_DECL	unsigned int _stack = 0x2000; WILDCARDS
# define dostime_t	dos_time_t	/* is Zortech out of step? */
# define _dos_gettime	dos_gettime
# define NO_MKSTEMP	1
# define NO_MKTEMP	1
#endif

#if defined(_OWCDOS__) && defined(MSDOS)	/* System: Watcom C V10.0 for the IBM-PC under MSDOS */
# define IBMPCDOS		1
# define MALLOC_CACHE	1	/* DGROUP gets full otherwise */
# define REALSTDC	1	/* close enough for us, but ZTCDOS doesn't define __STDC__ */
# ifdef M_I86LM		/* large memory model */
#  define NBUF		62	/* NBUF*JBUFSIZ must be less than 64 kB */
# else
#  define NBUF		3
#  define FAR_LINES	1	/* to squeeze larger files, distance Lines */
# endif
# define NO_MKSTEMP	1
# define NO_MKTEMP	1
#endif

#ifdef _BORLANDC__	/* System: Borland C/C++ (v3.1) for the IBM-PC under MSDOS */
# define IBMPCDOS		1
# define MALLOC_CACHE	1	/* DGROUP gets full otherwise */
# define REALSTDC	1	/* close enough for us, but not strict ANSI */
# ifdef __LARGE__
#  define NBUF		62	/* NBUF*JBUFSIZ must be less than 64 kB */
#  define FAR_LINES 1	/* to squeeze larger files, distance Lines */
# else
#  ifdef __MEDIUM__
#    define NBUF	3
#    define FAR_LINES 1	/* to squeeze larger files, distance Lines */
#  endif
# endif
# define STACK_DECL	unsigned int _stklen = 0x2000;		/* Borland's way of specifying stack size */
/* probably: # define NO_MKSTEMP	1 */
/* probably: # define NO_MKTEMP	1 */
#endif

/* All the systems marked with XXX_ are ones that even Jove 4.16
 * has not been tested on.  4.15 was the transition from implicit #ifdefs
 * scattered throughout the code to feature-based ifdefs that MUST be
 * enabled in sysdep.h in order to enable the relevant code.
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
# define BSD_SIGS	1	/* ??? */
# define USE_KILLPG	1
# define TERMIO		1	/* uses termio struct for terminal modes */
# define USE_GETHOSTNAME	1
# define USE_SELECT	1
#endif

#ifdef XXX_OLDMIPS	/* System: MIPS-SYSV, Irix before 3.3. */
/* Older MIPS (UMIPS-SYSV, anything other than their 4.3 port before
 * RISCOS4.x) and SGI 4D OSes (anything before Irix3.3) have BSD style wait,
 * and directory routines if you link -lbsd and define -I/usr/include/bsd on
 * the compile line. But they have SysV style signals.  Jove was ported to the
 * SGI 68K boxes once, but it the mods seem to have been lost.
 */
# define BSD_WAIT	1	/* Berkeley style sys/wait.h */
# define BSD_DIR	1	/* Berkeley style dirent routines */
#endif

#ifdef XXX_MSC51	/* System: Microsoft C 5.1 on IBM PC under DOS*/
/* This hasn't been tested recently.  Consider stealing ZTCDOS settings. */
# define IBMPCDOS		1
# define NO_PTRPROTO	1
# define REALSTDC	1	/* well, almost */
# ifdef M_I86LM		/* large memory model */
#  define NBUF		62	/* NBUF*JBUFSIZ must be less than 64 kB */
# else
#  define JSMALL		1
# endif
/* probably: # define NO_MKSTEMP	1 */
/* probably: # define NO_MKTEMP	1 */
#endif

#ifdef THINK_C	/* System: Think C version 5.0 on the Macintosh */
# define MAC 1
# define REALSTDC	1	/* we hope */
# define MALLOC_CACHE	1   /* Only 32K of static space on Mac, so... */
  typedef long	off_t;
# define USE_GETCWD	1
# define USE_INO	1	/* we fake it */
  typedef int	dev_t;
  typedef int	ino_t;
# define DIRECTORY_ADD_SLASH 1
# define NO_MKSTEMP	1	/* no mkstemp library routine */
# define NO_MKTEMP	1	/* no mktemp library routine */
# define NO_FCNTL	1	/* no <fcntl.h> header */
# define EOL	'\r'	/* end-of-line character for files */
# define WINRESIZE	1
# define AUTO_BUFS	1	/* slim down data segment */
#endif

