/************************************************************************
 * This program is Copyright (C) 1986 by Jonathan Payne.  JOVE is       *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

#define TUNED		/* don't touch this */

/*#define LSRHS		/* if this is Lincoln-Sudbury Regional High School */
/*#define MSDOS		/* if this is MSDOS */
#define BSD4_2		/* Berkeley 4.2 BSD */
/*#define BSD4_3	/* Berkeley 4.3 BSD */
/*#define SYSV		/* for (System III/System V) UNIX systems */
#ifdef BSD4_3
#   ifndef BSD4_2
#	define BSD4_2	/* 4.3 is 4.2 only different. */
#   endif
#endif


#ifdef MSDOS
#   define SMALL
#else			/* assume we're UNIX or something */
#   if vax || sel || sun || pyr || mc68000 || tahoe || iAPX286
#	define VMUNIX		/* Virtual Memory UNIX */
#	define BUFSIZ	1024
#	if iAPX286
#	    define NBUF	48
#	else
#	    define NBUF	64	/* number of disk buffers */
#	endif iAPX286
#   else
#	define SMALL
#	define BUFSIZ	512	/* or 1024 */
#	define NBUF	3
#   endif
#
/* #   define LOAD_AV	/* Use the load average for various commands.
#			   Do not define this if you lack a load average
#			   system call and kmem is read protected. */
#
#   define JOB_CONTROL	/* if you have job stopping */
#
#   ifdef JOB_CONTROL
#       define MENLO_JCL
#       define IPROCS	/* Interactive processes only work with JOB_CONTROL. */
#   endif
#
#   define SUBPROCS	/* only on UNIX systems (NOT INCORPORATED YET) */
#endif MSDOS

#ifdef SMALL
    typedef	short	disk_line;
#else
#   if iAPX286
	typedef long	disk_line;
#   else
	typedef	int	disk_line;
#   endif iAPX286
#endif SMALL

#ifndef SMALL
#   define ABBREV		/* word abbreviation mode */
#   define BACKUPFILES		/* enable the backup files code */
#   ifndef MSDOS
#       define BIFF		/* if you have biff (or the equivalent) */
#       define F_COMPLETION	/* filename completion */
#       define CHDIR		/* cd command and absolute pathnames */
#       define	KILL0	/* kill(pid, 0) returns 0 if proc exists */
#       define SPELL		/* spell words and buffer commands */
#       define ID_CHAR		/* include code to IDchar */
#       define WIRED_TERMS	/* include code for wired terminals */
#       define ANSICODES	/* extra commands that process ANSI codes */
#   endif
#   define LISP			/* include the code for Lisp Mode */
#   define CMT_FMT		/* include the comment formatting routines */
#endif SMALL

#if !sun && !iAPX286
#   define MY_MALLOC	/* use more memory efficient malloc (not on suns) */
#endif

#define DFLT_MODE	0666	/* file will be created with this mode */

#ifdef BSD4_3
#   define RESHAPING	/* enable windows to handle reshaping */
#endif

#ifdef BSD4_2			/* byte_copy(from, to, len) */
#   define	byte_copy bcopy	/* use fast assembler version */
#endif

#ifdef IPROCS
#   ifdef BSD4_2
#	define INPUT_SIG	SIGIO
#   else
#	define PIPEPROCS		/* do it with pipes */
#	define INPUT_SIG	SIGTINT
#   endif
#endif

#ifdef SYSV
#   define byte_copy(s2, s1, n)	memcpy(s1, s2, n)
#   define bzero(s, n)	memset(s, 0, n)
#   define index	strchr
#   define rindex	strrchr
#endif

/* These are here since they define things in tune.c.  If you add things to
   tune.c, add them here too, if necessary. */

#ifndef NOEXTERNS
extern char
	TmpFilePath[128],
	*d_tempfile,
	*p_tempfile,
	*Recover,
	*CmdDb,
	*Joverc,

#ifdef PIPEPROCS
	*Portsrv,
#endif

	Shell[],
	ShFlags[];
#endif NOEXTERNS
