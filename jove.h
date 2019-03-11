/************************************************************************
 * This program is Copyright (C) 1986-1994 by Jonathan Payne.  JOVE is  *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

/* jove.h header file to be included by EVERYONE */

#include <setjmp.h>
#ifndef TUNED
# include "tune.h"
#endif

#ifndef MAC
# include <sys/types.h>
# include <string.h>
#else
# include <types.h>
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

/* Since we don't use stdio.h, we may have to define NULL and EOF */

#ifndef NULL
# define NULL	0
#endif

#ifndef EOF
#define EOF	(-1)
#endif

#define private		static

typedef int	bool;
#define NO		0
#define YES		1

#define elemsof(a)	(sizeof(a) / sizeof(*(a)))	/* number of array elements */

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
extern bool	UpdModLine;	/* Does the mode line need to be updated? */

/* typedef pervasive structure definitions */

typedef struct window	Window;	/* wind.h */
typedef struct position	Bufpos;	/* buf.h */
typedef struct mark	Mark;	/* buf.h (not mark.h!) */
typedef struct buffer	Buffer;	/* buf.h */
typedef struct line	Line;	/* buf.h */
#ifdef FAR_LINES
typedef Line _far	*LinePtr;
#else
typedef Line	*LinePtr;
#endif
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

/* paths */

extern char
	ShareDir[FILESIZE],	/* path of machine-independent library */
	TmpDir[FILESIZE];	/* VAR: directory/device to store tmp files */

#ifdef SUBSHELL
extern char
	Shell[FILESIZE],	/* VAR: shell to use */
	ShFlags[16];	/* VAR: flags to shell */
#endif


/* setjmp/longjmp args for DoKeys() mainjmp */
#define FIRSTCALL	0
#define ERROR		1
#define COMPLAIN	2	/* do the error without a getDOT */
#define QUIT		3	/* leave this level of recursion */

extern jmp_buf	mainjmp;


extern char	NullStr[];


extern ZXchar
	peekchar,	/* holds pushed-back getch output */
	LastKeyStruck;	/* used by SelfInsert and friends */

extern int
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

#ifdef SUBSHELL
extern void	jcloseall proto((void));
#endif

extern SIGRESTYPE
	finish proto((int code)),	/* doesn't return at all! */
	win_reshape proto((int /*junk*/));

extern bool
	charp proto((void));

extern ZXchar
	getch proto((void)),
	kbd_getch proto((void)),
	waitchar proto((bool *slow));

extern void
	error proto((const char *, ...)),
	complain proto((const char *, ...)),
	raw_complain proto((const char *, ...)),
	confirm proto((const char *, ...)),
	SitFor proto((int delay)),
	pp_key_strokes proto((char *buffer, size_t size)),
	tty_adjust proto ((void)),
	Ungetc proto((ZXchar c)),
	kbd_ungetch proto((ZXchar c));

/* Commands: */

extern void
#ifdef JOB_CONTROL
	PauseJove proto((void)),
#endif
#ifdef SUBSHELL
	Push proto((void)),
#endif
	Recur proto((void)),
	ShowVersion proto((void));

/* Variables: */

extern ZXchar	IntChar;		/* VAR: ttysetattr sets this to generate QUIT */
extern bool	MetaKey;		/* VAR: this terminal has a meta key */
extern bool	OKXonXoff;		/* VAR: XON/XOFF can be used as ordinary chars */
extern int	UpdFreq;		/* VAR: how often to update modeline */

#ifdef BIFF
extern bool	DisBiff;		/* VAR: turn off/on biff with entering/exiting jove */
#endif
