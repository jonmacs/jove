/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* jove.h header file to be included by EVERYONE */

#ifndef TUNED
# include "tune.h"  /* must include first since it controls everything else */
#endif

#include <setjmp.h>
#include <string.h>
#include <errno.h>

#ifndef EWOULDBLOCK
/* older Unix, e.g. V7 */
# define RETRY_ERRNO(e)	((e) == EINTR || (e) == EAGAIN)
#else
# define RETRY_ERRNO(e) ((e) == EINTR || (e) == EAGAIN || (e) == EWOULDBLOCK)

#endif

#ifdef USE_STDIO_H
# include <stdio.h> /* for recover, setmaps */
#endif

#ifndef MAC
# include <sys/types.h>
#else
# include <types.h>
# include <time.h>	/* for time_t */
#endif

/* proto: macro to allow us to prototype any function declaration
 * without upsetting old compilers.
 */

#ifdef REALSTDC
# define    USE_PROTOTYPES  1
#endif

#ifdef USE_PROTOTYPES
# define proto(x)        x
# ifdef NO_PTRPROTO
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
#ifdef REALSTDC
#define	STDARGS	1
#endif

#ifdef STDARGS
# include <stdarg.h>
# define	va_init(ap, parmN)	{ va_start((ap), parmN); }
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

#ifdef REALSTDC

  typedef void	*UnivPtr;
  typedef const void	*UnivConstPtr;

#else /* !REALSTDC */

# ifndef const
#  define	const	/* Only in ANSI C.  Pity */
# endif
# ifndef volatile
#  define	volatile
# endif
  typedef char	*UnivPtr;
  typedef const char	*UnivConstPtr;

#endif /* !REALSTDC */

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
#ifdef MIPS_CC_BUG
# define UNMACRO(proc)	proc
#else
# define UNMACRO(proc)	(proc)
#endif

/* Since Jove does not use stdio.h (recover and setmaps do), we may have to define NULL and EOF */

#ifndef NULL
#define NULL	(void *)0
#endif

#ifndef EOF
#define EOF	(-1)
#endif

/* Pervasive data types and constants */

#ifndef CHAR_BIT
# define CHAR_BIT	8	/* factor to convert sizeof => bits */
#endif

#ifndef NCHARS
# define NCHARS 0400
#endif

#define NULL_DADDR	((daddr) 0)
#define DDIRTY		((daddr)1 << (sizeof(daddr)*CHAR_BIT - 1))	/* daddr dirty flag */

#define private		static

typedef int	bool;
#define NO		0
#define YES		1

#define elemsof(a)	(sizeof(a) / sizeof(*(a)))	/* number of array elements */

/* Safe-where-possible signal handling.
 *
 * SIGHANDLERTYPE: the type of a pointer to a signal handler
 * setsighandler: the best way to establish a signal handler
 * resetsighandler: the best way to re-establish a signal handler
 */

typedef SIGRESTYPE (*SIGHANDLERTYPE) ptrproto((int));

#ifdef POSIX_SIGS
extern SIGHANDLERTYPE setsighandler proto((int, SIGHANDLERTYPE));
# define resetsighandler(signo, handler)	/* nothing */
#else /* !POSIX_SIGS */
# ifdef USE_SIGSET
#  define setsighandler(signo, handler)	sigset((signo), (handler))
#  define resetsighandler(signo, handler)	/* nothing */
# else /* !USE_SIGSET */
   /* BSD_SIGS or default: use signal() */
#  define setsighandler(signo, handler)	signal((signo), (handler))
# endif /* !USE_SIGSET */
#endif /* !POSIX_SIGS */

#ifndef resetsighandler
 /* On some systems, the signal handler is left established,
  * but this is not the case with original UNIX signals.
  * This code adjusts to the system at hand, at fairly low cost.
  * This code is even used for BSD_SIGS, even though it should not
  * be needed: it compensates for mis-configuration.
  * Note: this routine will only work if every execution of a particular
  * call is for the same handler.  Furthermore, all executions should
  * be due to the handler being invoked by a signal.  These restrictions
  * ensure that the setting of the static variable indicates whether
  * signal handlers need to be re-established.
  */
# define resetsighandler(signo, handler)	{ \
	static SIGHANDLERTYPE	reset_handler; \
 \
	if (reset_handler != (handler)) \
		reset_handler = setsighandler((signo), (handler)); \
}
#endif

/* Principles of character representation in JOVE:
 *
 * - Only legal characters (excluding NUL) may be stored in the buffer.
 *
 * - Only legal characters will be found in the input stream, as
 *   delivered by getch and kbd_getch (the meta-key feature
 *   may cause invalid characters to be read, but they will be
 *   confined within kbd_getch).
 *
 * - Bad characters from a file should be discarded.  Perhaps a message
 *   should be generated.  NUL should be considered one of these bad
 *   characters.  [The elimination of these characters is the duty of
 *   callers of jgetc.]
 *
 * - The type of a string (ignoring const or volatile) should be "char *".
 *   This is the standard type specified by the ANSI/ISO C standard.
 *   As such, it is the type expected by the standard library.
 *
 * - In general, it is reasonable to use a plain char type
 *   to represent an individual character, avoiding the expense and
 *   fuss of widening.
 *
 *   + The character must be known not to be an EOF.
 *
 *   + The character must only be used in ways in which sign extension
 *     is known to not cause problems:
 *
 *     * The value is coerced into a char type (the most common example
 *       is assignment to a char; another is the search character in a
 *       call to strchr).  Of course, EOF must not appear in these
 *       contexts.
 *
 *     * The value is used for equality or inequality comparison where
 *       all operands are subject to identical char widening.  A
 *       special case of this is 7-bit ASCII char literals:
 *       they can be safely compared for equality or inequality with
 *       chars widened explicitly or implicitly.
 *
 * - ZXchar is the type for a character that has been widened without
 *   sign extension.  It can represent EOF distinctly from characters.
 *   The ZXC and ZXRC functions should be used to do the widening.
 *
 * - DAPchar is the type that results from the "default argument promotions"
 *   applied to the char type.  Since most of our function definitions are
 *   in the old style, formals declared in these definitions to be of type
 *   char should be declared to be of type DAPchar in any prototype (this is
 *   an arcane ANSI C rule).  The widening involved might be sign-extension
 *   or it might not, but we don't care because it will be immediately
 *   narrowed. A C implementation can legally make the type "unsigned int",
 *   but this is highly unlikely.  By extension, a value of this type may be
 *   used anywhere a char is needed.
 *
 * The following functions widen char type to an int, without sign
 * extension.  There are two kinds:
 *
 * ZXC(c): zero-extend an internal char.  It is presumed that
 *	the character is clean, but we may have to prevent sign extension.
 *
 * ZXRC(c): zero-extend a external (raw) char.  The purpose is to prevent
 *	sign extension, even if the char is invalid.
 */

typedef int ZXchar;	/* type for expanded char (possibly EOF) */

#ifndef DAPchar
# define DAPchar	int	/* DefaultArgPromotion(char) */
#endif

#ifndef UCHAR_ROOF
# define UCHAR_ROOF	256	/* better be true! */
#endif

#if NCHARS == 128
# ifndef ZXC
#  define ZXC(c)	(c)	/* identity op -- sign bit must be off */
# endif
#else
# if NCHARS == UCHAR_ROOF
#  ifndef ZXC
#   define ZXC(c)	((ZXchar) (unsigned char) (c))
#  endif
# else
   /* ??? */
# endif
#endif

#ifndef ZXRC
# define ZXRC(c)	((ZXchar) (unsigned char) (c))
#endif

/* Pervasive exports of other modules */

/* disp.c */
extern volatile bool	UpdModLine;	/* Does the mode line need to be updated? */

/* term.c: universal termcap-like declarations */

extern int
	SG,		/* number of magic cookies left by SO and SE */
	LI,		/* number of lines */
	ILI,		/* number of internal lines (LI - 1) */
	CO;		/* number of columns (CO <= MAXCOLS) */

extern bool
	TABS;		/* terminal supports tabs */

/* typedef pervasive structure definitions */

typedef struct window	Window;	/* wind.h */
typedef struct position	Bufpos;	/* buf.h */
typedef struct mark	Mark;	/* buf.h (not mark.h!) */
typedef struct buffer	Buffer;	/* buf.h */
#ifdef FAR_LINES
typedef struct line _far	*LinePtr;	/* buf.h */
#else
typedef struct line	*LinePtr;	/* buf.h */
#endif
typedef struct FileStruct	File;	/* fp.h */

#include "buf.h"
#include "io.h"
#include "dataobj.h"
#include "keymaps.h"
#include "argcount.h"
#include "util.h"

#ifndef NO_EXTERNS  /* setmaps.c does not need externs, messes up cross-compiles */
#include "externs.h"
#endif

#define FORWARD		1
#define BACKWARD	(-1)

/* jove.c exports: */

/* paths */

extern char
	ShareDir[FILESIZE],	/* VAR: directory path of machine-independent support files */
	LibDir[FILESIZE],	/* VAR: directory path of machine-dependent support programs */
	TmpDir[FILESIZE];	/* VAR: directory path to store tmp files, must exist */

#ifdef SUBSHELL
extern char
	Shell[FILESIZE],	/* VAR: shell to use */
	ShFlags[16];	/* VAR: flags to shell */
#endif


/* setjmp/longjmp args for DoKeys() mainjmp */
#define JMP_ERROR		1
#define JMP_COMPLAIN	2	/* do the error without a getDOT */
#define JMP_QUIT		3	/* leave this level of recursion */

extern jmp_buf	mainjmp;


#define IDX(c)   ((c)-'a')
#define IDXSZ    (IDX('z')+1)

extern char	NullStr[];


extern ZXchar
	peekchar,	/* holds pushed-back getch output */
	LastKeyStruck;	/* used by SelfInsert and friends */

extern int
	RecDepth,	/* recursion depth (used by disp.c for modeline) */
	SlowCmd;	/* depth of nesting of slow commands */

extern bool
	TOabort,	/* flag set by Typeout() */
	stickymsg,	/* the last message should stick around */
	InputPending,	/* is there input waiting to be processed? */
	Interactive;

#ifdef UNIX
extern bool
	InSlowRead;	/* Can we do a redisplay in a signal handler? */
#endif

extern char	*Inputp;

#ifdef WINRESIZE
# define PreEmptOutput()	(ResizePending || (SlowCmd == 0 && charp()))
# define CheapPreEmptOutput()	(ResizePending || (SlowCmd == 0 && InputPending))
#else
# define PreEmptOutput()	(SlowCmd == 0 && charp())
# define CheapPreEmptOutput()	(SlowCmd == 0 && InputPending)
#endif

#ifdef SUBSHELL
extern void	jcloseall proto((void));
#endif

extern SIGRESTYPE
	finish proto((int code)) NEVER_RETURNS,	/* doesn't return at all! */
	win_reshape proto((int /*junk*/));

extern bool
	charp proto((void));

extern ZXchar
	getch proto((void)),
	kbd_getch proto((void)),
	peek_or_mac_getch proto((void)),
	waitchar proto((void)),
	ask_ks proto((void));

extern void
	cmd_sync proto((void)),
	add_stroke proto((ZXchar)),
	error proto((const char *, ...)) NEVER_RETURNS,
	complain proto((const char *, ...)) NEVER_RETURNS,
	raw_complain proto((const char *, ...)),
	confirm proto((const char *, ...)),
	SitFor proto((int delay)),
	pp_key_strokes proto((char *buffer, size_t size)),
	tty_adjust proto ((void)),
	Ungetc proto((ZXchar c)),
	kbd_ungetch proto((ZXchar c)),
	dispatch_macros proto((void));

/* Commands: */

extern void
#ifdef JOB_CONTROL
	PauseJove proto((void)),
#endif
#ifdef SUBSHELL
	Push proto((void)),
#endif
	Recur proto((void)),
	TeachJove proto((void)),
	ShowVersion proto((void));

/* Variables: */

extern bool	MetaKey;		/* VAR: this terminal has a meta key */
extern bool	TimeDisplayed;		/* is time actually displayed in modeline? */
extern char	JoveFeatures[MAXCOLS];	/* VAR: list of compiled-in features */

#if defined(UNIX) || defined(MINGW)
extern char	JoveCompiled[MAXCOLS];	/* VAR: compile flags used to build this */
extern char	JoveLinked[MAXCOLS];	/* VAR: link flags used to build this */
#endif /* UNIX || MINGW */

#ifdef UNIX
extern int	UpdFreq;		/* VAR: how often to update modeline */
extern void	SetClockAlarm proto((bool unset));
#endif /* UNIX */

extern bool	SaveOnExit;		/* VAR: offer to save buffers on exit */
