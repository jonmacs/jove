/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#ifdef THINK_C
# define MAC 1
# define defined(x) (x)	/* take this out and you're in trouble... */
#endif

#if !(defined(MSDOS) || defined(MAC) || defined(__STDC__))
# define void int
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
# define YP_PASSWD	/* if you are a sun running the yellow pages */
# define VFORK		/* if you have vfork(2) */
# define JOB_CONTROL	/* if you have job stopping */
# ifdef JOB_CONTROL
#  define MENLO_JCL
# endif
# define KILL0		/* kill(pid, 0) returns 0 if proc exists */
#endif UNIX

#ifdef UNIX
# ifdef pdp11
#  define SMALL
#  define BUFSIZ	512	/* or 1024 */
#  define NBUF		3
# else
#  define VMUNIX		/* Virtual Memory UNIX */
#  define BUFSIZ	1024
#  ifdef iAPX286
#   define NBUF		48	/* NBUF*BUFSIZ must be less than 64 kB */
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

#ifdef UNIX
			/* pick your version of Unix */
# define BSD4_2		/* Berkeley 4.2 BSD */
# define BSD4_3		/* Berkeley 4.3 BSD and 2.10 BSD */
/*# define SYSV		/* for (System III/System V) UNIX systems */
/*# define SYSVR2	/* system 5, rel. 2 */
			/* M_XENIX is defined by the Compiler */
#endif /* UNIX */

#ifdef SYSVR2
# ifndef SYSV
#  define SYSV	/* SYSV is a subset of SYSVR2 */
# endif
#endif

#ifdef BSD4_3
# ifndef BSD4_2
#  define BSD4_2	/* 4.3 is 4.2 only different. */
# endif
#endif

#ifdef M_XENIX
# define iAPX286 1	/* we have segments */
#endif

#ifdef MSDOS
# ifdef M_I86LM		/* large memory model */
#  define NBUF 		64
# else
#  define NBUF 		3
#  define SMALL
# endif
# define BUFSIZ		512		/* or 1024 */
#endif

#if !sun && !iAPX286
# define MY_MALLOC	/* use more memory efficient malloc (not on suns) */
#endif

#if (defined(BSD4_3) || defined(MAC))
# define RESHAPING	/* enable windows to handle reshaping */
#endif

#ifdef BSD4_2			/* byte_copy(from, to, len) */
# define byte_copy	bcopy	/* use fast assembler version */
#endif

#if defined(IPROCS) && !defined(BSD4_2)
# define PIPEPROCS		/* do it with pipes */
#endif

#if defined(SYSV) || defined(MSDOS) || defined(M_XENIX)
# define byte_copy(s2, s1, n)	memcpy(s1, s2, n)
# define bzero(s, n)		memset(s, 0, n)
# define index	strchr
# define rindex	strrchr
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

#if defined(BSD4_2)
extern long	SigMask;

# define SigHold(s)	sigblock(SigMask |= sigmask(s))
# define SigRelse(s)	sigsetmask(SigMask &= ~sigmask(s))
#endif
