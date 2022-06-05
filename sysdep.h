/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/*
 * System Feature Selection: describe OS and C system to JOVE.  See sysdep.doc
 * for a better description of what the different #define feature symbols
 * mean.  If you add new ones, please keep the syntax of the first #ifdef, so
 * that "grep System: sysdep.h" catches the first line of all symbols.
 * See old/sysdep.h for historical symbols for untested systems that once
 * worked.
 */

/* The modern *BSD family all have slightly different quirks */
#ifdef NetBSD
/* System: modern NetBSD, sigh, TIOCREMOTE does not work, see iproc.c comment */
# define XBSD		1
# define NO_TIOCREMOTE	1
#endif

#if defined(FreeBSD) || defined(DragonFly)
/* System: modern FreeBSD, needs HAVE_LIBUTIL_H */
# define XBSD		1
# define HAVE_LIBUTIL_H 1
# define SPELL		"aspell list < %s | sort -u"
#endif

#if defined(OpenBSD) || defined(Darwin) || defined (XBSD)
/* System: modern OpenBSD, Darwin Mac OSX */
# define BSDPOSIX_STDC	1
# define USE_OPENPTY	1
#endif

#if defined(SunOS)
/* System: SunOS 5.1 aka Solaris 2.1 onwards, including Illumos/Joyent/OpenSolaris/OpenIndiana */
# define SYSVR4		1
#endif

#if defined(XLINUX)
/*
 * System: Some Linux e.g. Debian 9 and earlier.  Very old Linux (e.g. before
 * RedHat6) used BSD-compatible pty handling so BSDPOSIX_STDC is better for
 * those. Middle-aged Linux (e.g. upto Debian 9) seemed to work better with
 * SYSVR4 and _XOPEN_SOURCE defined.  Modern Linux (e.g. Debian 10 on) works
 * better with GLIBCPTY, since they have openpty in libutil, and pty.h.
 */
# define SYSVR4		1
# ifndef _XOPEN_SOURCE
#  define _XOPEN_SOURCE	500
# endif
# define SPELL		"aspell list < %s | sort -u"
#endif

#if defined(CYGWIN) || defined(CYGWIN32)
/* System: Cygwin POSIX-like environment on Windows (see README.cyg) */
# define FILENAME_CASEINSENSITIVE	1
# define GLIBCPTY	1
# define O_TRUNC_BROKEN	1   /* see fp.c */
# define SPELL		"aspell list < %s | sort -u"
# define JTC		1 /* no real point using curses for Cygwin, surely?! */
#endif

#if defined(Linux) || defined(GNU) || defined(GLIBCPTY)
/*
 * modern glibc e.g. Linux, Cygwin) provides openpty and pty.h
 * so this is also Linux alternative. Also GNU Debian Hurd.
 */
# define USE_OPENPTY	1	/* older Cygwin may not have openpty? */
# define HAVE_PTY_H	1
# define BSDPOSIX_STDC	1
#endif

#ifdef MINGW		/* System: MinGW cross-compilation for Windows WIN32 (see README.w32) */
# define WIN32		1
# define POSIX_UNISTD	1
# define NO_MKSTEMP	1   /* MKSTEMP on Windows unlinks the filename, which Jove uses for filters */
#endif

#ifdef OWCDOS	/* System: Open Watcom C 1.9 for x86 running MSDOS (see README.dos) */
# define IBMPCDOS	1
# define REALSTDC	1	/* close enough for us, but ZTCDOS doesn't define __STDC__ */
# define FULL_UNISTD	1
#endif

#ifdef _MSC_VER	/* System: Microsoft C for the IBM-PC under MSDOS or WIN32 (see README.dos or README.w32) */
/* 4.16.0.38 tested under VC++ 5.0 / VS 97 */
/* 4.16.0.62 tested under Visual C++ 6.0 SP5 */
/* 4.17.x.x tested under Visual Studio 2019 Community Edition */
# if defined(_WIN32) && !defined(WIN32)
#  define WIN32 _WIN32
# endif
# ifndef WIN32 /* ! WIN32 => MSDOS, worked pre-MSVC 7.x */
#  define IBMPCDOS		1
# endif
# define NO_MKSTEMP	1
# define _POSIX_	1	/* suppresses MS's min and max in VC++ 5.0 */
# define jmode_t	int	/* no mode_t on WIN32 */
#endif

/**************************************************************************/
/* Some very common collectons of capabilities, used by many defs above   */

#ifdef BSDPOSIX_STDC	/* Same as BSDPOSIX, but with a Standard enough C */
/* System: BSDI, 386BSD, BSD4.4, NetBSD -- BSDPOSIX_STDC */
/* System: Old LINUX (MCC-Interim release) -- BSDPOSIX_STDC */
# define REALSTDC	1
# define BSDPOSIX	1
#endif

#ifdef BSDPOSIX	/* System: Posix system with BSD flavouring for ptys */
/* System: SunOS4.1.3, DEC Ultrix 4.2 -- BSDPOSIX */
/* System: DEC OSF/1 V1.3 -- BSDPOSIX + NO_TIOCREMOTE + NO_TIOCSIGNAL */
# define TERMIOS	1
# define USE_GETCWD	1
# define FULL_UNISTD	1
# define USE_SELECT	1
# if !defined(PIPEPROCS) && !defined(NO_IPROCS)	/* useful to test PIPEPROCS even on pty platforms */
#  define PTYPROCS	1
#  define BSD_PTYS	1	/* beware security flaw! */
# endif
# define POSIX_PROCS	1
# define POSIX_SIGS	1
# define JOB_CONTROL	1
# define BSD_SETPGRP	1
# define USE_KILLPG	1
# define USE_GETPWNAM	1
# define USE_GETHOSTNAME	1
# define USE_FSYNC	1
# define USE_FSTAT	1
# define USE_FCHMOD	1
# define HAS_SYMLINKS	1
# ifndef ISO_8859_1 /* fudge for __convex__ (see above) */
#  define USE_CTYPE	1
# endif
#endif

#ifdef SYSVR4	/* System: System V, Release 4 and derivatives */
/* System: Consensys V4 -- use SYSVR4 and GRANTPT_BUG */
/* System: DEC OSF/1 V2.0 or later -- use SYSVR4 */
/* System: DEC OSF R1.3MK -- use SYSVR4 */
/* System: Digital UNIX V4.0 and later -- use SYSVR4 and GRANTPT_BUG */
/* System: Solaris 2.0, SunOS 5.0 -- use SYSVR4 and GRANTPT_BUG */
/* System: Solaris 2.x onwards, SunOS 5.x, OpenIndiana/Illumos/Joyent -- use SYSVR4 */
/* Note: some versions of System V Release 4 have a bug in that affects
 * interactive processes.  Examples include Consensys V4 and SunOS 5.0
 * also known as Solaris 5.0.  See the description of GRANTPT_BUG in
 * sysdep.doc.  It turns out that this bug is documented as a feature
 * in "The Single UNIX Specification", Version 2!
 */
# define TERMIOS	1
# define USE_GETCWD	1
# define FULL_UNISTD	1
# define USE_SELECT	1
# if !defined(PIPEPROCS) && !defined(NO_IPROCS)	/* useful to test PIPEPROCS even on pty platforms */
#  define PTYPROCS	1
#  define SVR4_PTYS	1
# endif
# define POSIX_PROCS	1
# define POSIX_SIGS	1
# define JOB_CONTROL	1
# define USE_UNAME	1
# define USE_GETPWNAM	1
# define USE_FSYNC	1
# define USE_FSTAT	1
# define USE_FCHMOD	1
# define HAS_SYMLINKS	1
# define REALSTDC	1
# define USE_CTYPE	1
#endif

/**************** Common Characteristics ****************/

#ifdef pdp11
/* On the PDP-11, UNIX allocates at least 8K for the stack.
 * In order not to waste this space, we allocate
 * a bunch of buffers as autos.
 */
# define AUTO_BUFS	1
#endif

#ifdef IBMPCDOS	/* Common characteristics for IBM-PC MS-DOS systems. */
# ifndef MSDOS
#  define MSDOS	1
# endif
# define PCNONASCII	0xFF	/* prefix for peculiar IBM PC key codes */
# define NO_JSTDOUT	1	/* don't use jstdout */
# define CODEPAGE437	1	/* Code Page 437 English display characters */
# define PCSCRATTR	1	/* exploit IBMPC screen attributes */
# define HIGHLIGHTING	1	/* highlighting is used for mark and scrollbar */
# define MALLOC_CACHE	1	/* DGROUP gets full otherwise */
# define JSMALL		1	/* less than 64K lines fit in memory anyway */
# define FAR_LINES	1	/* to squeeze larger files, use far line pointers */
# if defined(M_I86SM) || defined(__SMALL__) || defined(M_I86CM) || defined(__COMPACT__)
/* try small or compact memory model: currently 20K over the 64K code+data limit, so not really viable */
#  define BAREBONES     1
# endif
# if defined(M_I86LM) || defined(__LARGE__)	/* large memory model */
#  define LG_JBUFSIZ	11	/* so JBUFSIZ (and max line len) 2048 chars */
#  define NBUF		30	/* NBUF*JBUFSIZ must be less than 64K. Is this true even if MALLOC_CACHE is set? */
# endif
#endif

#ifdef MSDOS	/* Common characteristics for MS-DOS systems. */
# define MSDOS_PROCS	1	/* spawn et al */
# define FILENAME_CASEINSENSITIVE 1
# define USE_CRLF 1
# define DIRECTORY_ADD_SLASH 1
# define MSFILESYSTEM 1
#endif

#ifdef WIN32	/* Common characteristics for WIN32 systems. */
# define REALSTDC	1	/* MS C only defines __STDC__ if you use /Za */
# define WINRESIZE	1
# define PCNONASCII	0xFF	/* prefix for peculiar IBM PC key codes */
# define NO_JSTDOUT	1	/* don't use jstdout */
# define CODEPAGE437	1	/* Code Page 437 English display characters */
# define PCSCRATTR	1	/* exploit IBMPC screen attributes */
# define HIGHLIGHTING	1	/* highlighting is used for mark and scrollbar */
# define MSDOS_PROCS	1	/* spawn et al */
# define FILENAME_CASEINSENSITIVE 1
# define USE_CRLF	1
# define DIRECTORY_ADD_SLASH 1
# define MSFILESYSTEM	1
#endif

/* The operating system (MSDOS, WIN32, or MAC) must be defined by this point. */
#if !(defined(MSDOS) || defined(WIN32) || defined(MAC))
# define UNIX	1	/* default to UNIX */
#endif

#ifdef UNIX	/* Common characteristics for UNIX systems. */
/* Our defaults tend to be conservative and lean towards pure SYSV */
# define USE_INO	1
# define TERMCAP	1
# define NCURSES_BUG	1   /* almost certainly safe anyway */
# define WINRESIZE	1
# define MOUSE		1
# define MALLOC_CACHE	1
# if !(defined(USE_PWD) || defined(USE_GETCWD) || defined(USE_GETWD))
#  define USE_GETWD     1
# endif
# if !(defined(NO_IPROCS) || defined(PIPEPROCS) || defined(PTYPROCS))
#  define PIPEPROCS	1	/* use pipes */
# endif
# if !defined(TERMIOS) && !defined(SGTTY)
#  define TERMIO	1	/* uses termio struct for terminal modes */
# endif
/* At the moment, the PTY code mandates having select().  One day, this might
 * change.
 */
# if defined(PTYPROCS) && !defined(USE_SELECT)
   sysdep.h: Sorry, PTYPROCS requires the select() system call.  You must
   either define USE_SELECT or undefine PTYPROCS.
# endif
# if defined(SIGCLD) && !defined(SIGCHLD)
#  define SIGCHLD	SIGCLD
# endif
# ifndef USE_EXIT
#  define EXIT _exit	/* so linker does not drag in all of stdio for static linking */
# endif
#endif /* UNIX */

#ifndef EXIT
# define EXIT	exit
#endif

/* lint suppression macros; GCC requires use of extensions! Clang mimics. */
#if !defined(GCC_LINT) && (defined(__GNUC__) || defined(__clang__))
# define GCC_LINT
#endif

#ifdef GCC_LINT
# define UNUSED(x)	x __attribute__ ((unused))
# define NEVER_RETURNS	__attribute__ ((noreturn))
#else /* !GCC_LINT */
# define UNUSED(x)	x
# define NEVER_RETURNS
#endif /* !GCC_LINT */

/*************************************************************************
 *
 * The things below here aren't meant to be tuned, but are included here
 * because they're dependent on the things defined earlier in the file.
 */
#ifdef USE_BCOPY
# define byte_copy(from, to, len)	bcopy((UnivConstPtr)(from), (UnivPtr)(to), (size_t)(len))
# define byte_move(from, to, len)	byte_copy(from, to, len)
# define byte_zero(s, n)	bzero((UnivPtr)(s), (size_t)(n))
#endif

#ifndef byte_copy
# ifdef USE_MEMORY_H
#  include <memory.h>
# endif
# define byte_copy(from, to, count)	memcpy((UnivPtr)(to), (UnivConstPtr)(from), (size_t)(count))
# define byte_move(from, to, count)	memmove((UnivPtr)(to), (UnivConstPtr)(from), (size_t)(count))
# define byte_zero(s, n)		memset((UnivPtr)(s), 0, (size_t)(n))
#endif

#ifdef USE_INDEX
# define strchr		index
# define strrchr	rindex
#endif

#ifdef FULL_UNISTD
# define POSIX_UNISTD	1
#endif

/* JSSIZE_T: result type of read() and write() */
#ifdef FULL_UNISTD
# define JSSIZE_T    ssize_t
#endif

#ifndef JSSIZE_T
# define JSSIZE_T    int
#endif

/* jmode_t: the type for file modes
 * Really old systems might use "int" or perhaps "unsigned".
 */
#ifndef jmode_t
# define jmode_t mode_t
#endif

/* Determine if really ANSI C */
#ifdef __STDC__
# if __STDC__ > 0
#  define REALSTDC 1
# endif
#endif

#ifndef SIGRESTYPE	/* default to void, correct for most modern systems */
# define SIGRESTYPE	void
# define SIGRESVALUE	/*void!*/
#endif

#ifndef EOL
# define EOL	'\n'	/* end-of-line character for files */
#endif

#ifdef JSMALL
  /*
   * On small memory/no-VM machines, save as much data space
   * as we can.  Going to hit either Out of lines, Tmp file
   * too large, or Out of memory (for open files) at some
   * point, alas.
 */
typedef unsigned short	daddr;    /* index of line contents in tmp file, see temp.h. */
# define LG_FILESIZE	7	/* log2 maximum path length (including '\0'): currently, 2+1+64+3+1+3+1 == 80 ought to be OK */
# define MAXCOLS	132	/* maximum number of columns */
# define MAXTTYBUF	512	/* maximum size of output terminal buffer */
# ifndef LG_JBUFSIZ
#  define LG_JBUFSIZ	9	/* temp file block size, also maximum line length. 512 bytes is the traditional UNIX block size */
# endif
# ifndef NBUF
#  define NBUF		3	/* number of temp file blocks to cache in memory, NBUF*JBUFSIZ is big part of JOVE memory footprint */
# endif

#else
  /*
   * On most modern machines (VM, or lots of memory), use
   * slightly more generous buffer sizes to improve speed
   * and/or convenience.
   */
typedef unsigned long	daddr;    /* index of line contents in tmp file, see temp.h. */
# define LG_FILESIZE	8	/* log2 maximum path length (including '\0') */
# define MAXCOLS	512	/* maximum number of columns */
# define MAXTTYBUF	2048	/* maximum size of output terminal buffer */
# ifndef LG_JBUFSIZ
#  define LG_JBUFSIZ	15	/* temp file block size, also maximum line length, 32K seems better for modern VM machines */
# endif
# ifndef NBUF
#  define NBUF		64	/* number of temp file blocks to cache in memory, NBUF*JBUFSIZ is big part of JOVE memory footprint */
# endif

#endif /* !JSMALL */

#define JBUFSIZ			(1 << LG_JBUFSIZ)
#define FILESIZE		(1 << LG_FILESIZE)
