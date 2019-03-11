/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/* jove.h header file to be included by EVERYONE */

#include <setjmp.h>
#ifndef	TUNED
# include "tune.h"
#endif

#ifndef	MAC
# include <sys/types.h>
# include <string.h>
#else
# include <types.h>
#endif

/* proto: macro to allow us to prototype any function declaration
 * without upsetting old compilers.
 */

#ifdef	REALSTDC
# define    USE_PROTOTYPES  1
#endif

#ifdef	USE_PROTOTYPES
# define proto(x)        x
# ifdef	NO_PTRPROTO
   /* on these systems, a prototype cannot be used for a pointer to function */
#  define ptrproto(x)		()
# else
#  define ptrproto(x)		x
# endif
#else
# define proto(x)		()
# define ptrproto(x)		()
#endif

/* There are two ways to handle functions with a variable number of args.
 * The old portable way uses varargs.h.  The way sanctioned by ANSI X3J11
 * uses stdarg.h.
 */
#ifdef	REALSTDC
#define	STDARGS	1
#endif

#ifdef	STDARGS
# include <stdarg.h>
# define	va_init(ap, parmN)	{ va_start((ap), (parmN)); }
#else
# include <varargs.h>
# define	va_init(ap, parmN)	{ va_start((ap)); }
#endif

/* ANSI Goodies and their substitutes
 *
 * const: readonly type qualifier
 *
 * volatile: type qualifier indicating one of two kinds of magic.
 * 1. This object may be modified by an event unknown to the implementation
 *    (eg. asynchronous signal or memory-mapped I/O device).
 * 2. This automatic variable might be modified between a setjmp()
 *    and a longjmp(), and we wish it to have the correct value after
 *    the longjmp().  This second meaning is an X3J11 abomination.
 * So far, only the second meaning is used.
 *
 * UnivPtr: universal pointer type
 *
 * UnivConstPtr: universal pointer to const
 */

#ifdef	REALSTDC

  typedef void	*UnivPtr;
  typedef const void	*UnivConstPtr;

#else	/* !REALSTDC */

# ifndef const
#  define	const	/* Only in ANSI C.  Pity */
# endif
# ifndef volatile
#  define	volatile
# endif
  typedef char	*UnivPtr;
  typedef const char	*UnivConstPtr;

#endif	/* !REALSTDC */

/* According to the ANSI standard for C, any library routine may
 * be defined as a macro with parameters.  In order to prevent
 * the expansion of this macro in a declaration of the routine,
 * ANSI suggests parenthesizing the identifier.  This is a reasonable
 * and legal approach, even for K&R C.
 *
 * A bug in the MIPS compiler used on MIPS, IRIS, and probably other
 * MIPS R[23]000 based systems, causes the compiler to reject
 * these declarations (at least at the current time, 1989 August).
 * To avoid this bug, we conditionally define and use UNMACRO.
 */
#ifdef	MIPS_CC_BUG
# define UNMACRO(proc)	proc
#else
# define UNMACRO(proc)	(proc)
#endif

/* Since we don't use stdio.h, we may have to define NULL and EOF */

#ifndef	NULL
# define NULL	0
#endif

#ifndef	EOF
#define EOF	(-1)
#endif

#define private		static

typedef int	bool;
#define NO		0
#define YES		1

/* Pervasive exports of other modules */

/* disp.c */
extern bool	UpdModLine;	/* Does the mode line need to be updated? */

/* typedef pervasive structure definitions */

#ifdef	IPROCS
typedef struct process	Process;	/* iproc.h */
#endif
typedef struct window	Window;	/* wind.h */
typedef struct position	Bufpos;	/* buf.h */
typedef struct mark	Mark;	/* buf.h (not mark.h!) */
typedef struct buffer	Buffer;	/* buf.h */
typedef struct line	Line;	/* buf.h */
typedef struct FileStruct	File;	/* fp.h */

#include "buf.h"
#include "io.h"
#include "dataobj.h"
#include "keymaps.h"
#include "argcount.h"
#include "util.h"

#include "externs.h"

#define FORWARD		1
#define BACKWARD	(-1)

/* jove.c exports: */

extern jmp_buf	mainjmp;

/* setjmp/longjmp args for DoKeys() mainjmp */
#define FIRSTCALL	0
#define ERROR		1
#define COMPLAIN	2	/* do the error without a getDOT */
#define QUIT		3	/* leave this level of recursion */

extern char	NullStr[];

extern int
	LastKeyStruck,	/* used by SelfInsert and friends */
	RecDepth;	/* recursion depth (used by disp.c for modeline) */

extern bool
	TOabort,	/* flag set by Typeout() */
	errormsg,	/* last message was an error message
			   so don't erase the error before it
			   has been read */
	InputPending,	/* nonzero if there is input waiting to
			   be processed */
	Interactive,
	inIOread;	/* so we know whether we can do a redisplay. */

extern char	*Inputp;

#ifdef USE_SELECT
extern fd_set	global_fd;
extern int	global_maxfd;
#endif

#ifdef	SUBSHELL
extern void	jcloseall proto((void));
#endif

extern SIGRESTYPE
	finish proto((int code)),	/* doesn't return at all! */
	win_reshape proto((int /*junk*/));

extern int
	charp proto((void)),
	getch proto((void)),
	jgetchar proto((void)),
	waitchar proto((int *slow));

extern void
	error proto((const char *, ...)),
	complain proto((const char *, ...)),
	raw_complain proto((const char *, ...)),
	confirm proto((const char *, ...)),
	SitFor proto((int delay)),
	pp_key_strokes proto((char *buffer, size_t size)),
	tty_reset proto ((void)),
	Ungetc proto((int c));

/* Commands: */

extern void
#ifdef	JOB_CONTROL
	PauseJove proto((void)),
#endif
#ifdef	SUBSHELL
	Push proto((void)),
#endif
	Recur proto((void)),
	ShowVersion proto((void));

/* Variables: */

extern int	IntChar;		/* ttysets this to generate QUIT */
extern bool	MetaKey;		/* this terminal has a meta key */
extern bool	OKXonXoff;		/* disable start/stop characters */
extern int	UpdFreq;		/* how often to update modeline */
