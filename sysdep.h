/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#ifdef THINK_C
# define MAC 1
# define defined(x) (x)	/* take this out and you're in trouble... */
typedef int size_t;
#endif

#if defined(MAC) || defined(MSDOS)
    extern int	errno;
#endif	/* MAC */

#if !(defined(MSDOS) || defined(MAC) || defined(__STDC__))
# define void int
#endif

#ifndef	__STDC__
#define	const
#endif

/* The operating system (MSDOS or MAC) must be defined by this point.
   IBMPC is defined in the Makefile. All MAC defines should be
   numerical (i.e. #define MAC 1) so that defined() will work. */

#if !(defined(MSDOS) || defined(MAC))
# define UNIX
#endif

#ifdef UNIX
# if !sun
    extern int	errno;
# endif
# if defined(sun) || defined(__sun__)
#  define YP_PASSWD	/* if you are a sun running the yellow pages */
#  ifdef SUNOS4	/* gone to void */
#   define SIGRESULT	void
#   define SIGRETURN	{ return; }
#  endif
# endif
# define KILL0		/* kill(pid, 0) returns 0 if proc exists */
# define BSD4_2		/* True enough for Jove's purposes */
#endif /* UNIX */

#ifdef UNIX
# ifdef pdp11
#  define SMALL
#  define JBUFSIZ	512	/* or 1024 */
#  define NBUF		3
# else
#  define VMUNIX		/* Virtual Memory UNIX */
#  define JBUFSIZ	1024
#  ifdef iAPX286
#   define NBUF		48	/* NBUF*JBUFSIZ must be less than 64 kB */
#  else
#   define NBUF	64	/* number of disk buffers */
#  endif /* iAPX286 */
# endif
#endif

#ifdef SMALL
  typedef unsigned short	daddr;
#else
# if defined(iAPX286) || defined(MSDOS) || defined(MAC)
   typedef long	daddr;
# else
   typedef	int	daddr;
# endif /* iAPX286 */
#endif /* SMALL */


#ifdef SVR3
# define SYSVR3		/* system 5, rel. 3 */
#endif

#ifdef SVR2
# define SYSVR2		/* system 5, rel. 2 */
#endif

#ifdef BSD
# define BSD4_2		/* Berkeley 4.2 BSD or later */
#endif

#if defined(ultrix) || defined(__ultrix__)
# define ULTRIX
# define BSD4_2		/* True enough for Jove's purposes */
# define SIGRESULT  void
# define SIGRETURN  {return;}
#endif

/* M_XENIX is defined by the Compiler */
/* SYSV should be defined for (System III/System V) UNIX systems */

#ifdef SYSVR3
# ifndef SYSVR2
#  define SYSVR2	/* SYSVR2 is a subset of SYSVR3 */
# endif
# define	SIGRESULT	void
# define	SIGRETURN	{ return; }
# ifndef SIGCHLD
#  define	SIGCHLD		SIGCLD
# endif
#endif

#ifdef SYSVR2
# ifndef SYSV
#  define SYSV	/* SYSV is a subset of SYSVR2 */
# endif
#endif

#ifdef M_XENIX
# define iAPX286 1	/* we have segments */
# define BSD_DIR
#endif

#ifdef MSDOS
# ifdef M_I86LM		/* large memory model */
#  define NBUF 		64
# else
#  define NBUF 		3
#  define SMALL
# endif
# define JBUFSIZ		512		/* or 1024 */
#endif

#ifdef BSD4_2			/* byte_copy(from, to, len) */
# define byte_copy	bcopy	/* use fast assembler version */
# define byte_zero	bzero
# define strchr	index
# define strrchr	rindex
# define BSD_SIGS	/* Berkeley style signals */
# define BSD_WAIT	/* Berkeley style sys/wait.h */
# define WAIT3		/* Berkeley style wait3() */
# define BSD_DIR		/* Berkeley style dirent routines */
# define VFORK		/* if you have vfork(2) */
# define JOB_CONTROL	/* if you have job stopping */
#endif

#ifdef JOB_CONTROL
# define MENLO_JCL
#endif

#ifdef A_UX		/* A/UX on a MacII */
/* It might be better to define POSIX compatibility and try that. Oh well! */
# define BSD_WAIT	/* Berkeley style sys/wait.h */
# define BSD_DIR	/* Berkeley style dirent routines */
# define WAIT3		/* Berkeley style wait3() */
# define BSD_SIGS	/* Berkeley style signals */
# define SYSV		/* System V everything else */
#endif

#if defined(mips) || defined(__mips__)
/*
 * MIPS and SGI boxes have BSD style wait, and directory routines if you link
 * -lbsd and define -I/usr/include/bsd on the compile line. But they have SysV
 * style signals. When compiling for MIPS under RISCos4.0, just say
 * BSD -- this is for older UMIPS systems.
 */
# ifndef ULTRIX
   /* Not a DECstation 3100 or suchlike */
#  define BSD_WAIT	/* Berkeley style sys/wait.h */
#  define BSD_DIR	/* Berkeley style dirent routines */
# else
#  undef ULTRIX		/* Only needed it for this test */
# endif
# if defined(sgi) || defined(__sgi__)
#  define WAIT3		/* Berkeley style wait3() */
# endif
#endif

#ifndef VFORK
# define vfork	fork
#endif

#ifndef BSD4_2
# define PIPEPROCS		/* if IPROCS selected, use pipes */
# define KBDSIG	SIGEMT
#endif

#if defined(SYSV) || defined(MSDOS) || defined(M_XENIX)
#include <memory.h>
# define byte_copy(s2, s1, n)	memcpy((s1), (s2), (n))
# define byte_zero(s, n)		memset((s), 0, (n))
#endif

#ifndef	SIGRESULT
# define	SIGRESULT	int
# define	SIGRETURN	{ return 0; }
#endif

#ifndef BSD4_2
# ifdef MENLO_JCL
#  define signal	sigset
# endif /* MENLO_JCL */
#endif

#if !(defined(IBMPC) || defined(MAC))
# define TERMCAP
# define ASCII
#endif

#ifdef ASCII	/* seven bit characters */
# define NCHARS 0200
#else
# define NCHARS 0400
#endif

#define CHARMASK (NCHARS - 1)

#ifndef MSDOS
# define FILESIZE	256
#else /* MSDOS */
# define FILESIZE	64
#endif /* MSDOS */

#if defined(BSD_SIGS)
extern long	SigMask;

# define SigHold(s)	sigblock(SigMask |= sigmask((s)))
# define SigRelse(s)	sigsetmask(SigMask &= ~sigmask((s)))
#else
# define SigHold(s)	sighold(s)
# define SigRelse(s)	sigrelse(s)
# define killpg(pid, sig)	kill(-(pid), (sig))
#endif
