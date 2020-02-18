/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* UNIX Library/System Routine Emulations for Macintosh (mac.c) */

#ifdef MAC

extern int
	creat proto((const char *, jmode_t)),
	/* open may have an optional third argument, promo(jmode_t) mode */
	open proto((const char */*path*/, int /*flags*/, ...)),
	close proto((int)),
	unlink proto((const char *)),
	chdir proto((const char *));

extern SSIZE_T
	read proto((int /*fd*/, UnivPtr /*buf*/, size_t /*nbytes*/)),
	write proto((int /*fd*/, UnivConstPtr /*buf*/, size_t /*nbytes*/));

extern off_t	lseek proto((int /*fd*/, off_t /*offset*/, int /*whence*/));
extern time_t	time proto((time_t *));

extern void
	menus_off proto((void));	/* called by real_ask, findcom, waitchar */

#endif /* MAC */

/*==== Declarations of Library/System Routines ====*/

#ifndef REALSTDC
extern int	errno;	/* Redundant if declared in <errno.h> -- DHR */
#endif
extern char *strerror proto((int));	/* errno.h or string.h? */

/* General Utilities: <stdlib.h> */

#ifdef REALSTDC
#include <stdlib.h>
#endif

extern int	abs proto((int));

extern void	abort proto((void));
extern void	exit proto((int));

extern int	atoi proto((const char */*nptr*/));

extern void	qsort proto((UnivPtr /*base*/, size_t /*nmemb*/,
	size_t /*size*/, int (*/*compar*/)(UnivConstPtr, UnivConstPtr)));

extern char	*getenv proto((const char *));
extern int	system proto((const char *));

extern void
	free proto((UnivPtr));

extern UnivPtr
	calloc proto((size_t, size_t)),
	malloc proto((size_t)),
	realloc proto((UnivPtr, size_t));

/* Date and Time <time.h> */

extern time_t	time proto((time_t */*tloc*/));
extern char	*ctime proto((const time_t *));

/* UNIX */

#ifdef MSC51
#define const	/* the const's in the following defs conflict with MSC 5.1 */
#endif

#ifndef NO_FCNTL
# include <fcntl.h>
#endif

#ifndef O_RDONLY	/* in POSIX <fcntl.h> */
# define O_RDONLY	0
# define O_WRONLY	1
# define O_RDWR		2
#endif

#ifndef O_BINARY	/* needed for MSDOS and perhaps others */
# define O_BINARY	0	/* fake if missing with value harmless when ORed */
#endif
#ifndef O_CLOEXEC
# define O_CLOEXEC	0
#endif

#ifdef POSIX_UNISTD
# include <unistd.h>
# if _POSIX_VERSION < 199009L	/* defined in <unistd.h>: can't test earlier */
   /* ssize_t is not defined in original POSIX.1
    * Surprise: NetBSD defines it, even though it claims to be old POSIX.1!
    * To dodge this problem, we define ssize_t as a macro, not a typedef.
    */
#  define ssize_t	int
# endif
#else /* !POSIX_UNISTD */

extern int	chdir proto((const char */*path*/));

/* POSIX, System Vr4, MSDOS, and our Mac code specify getcwd.
 * System Vr4 (sometimes?) types the second argument "int"!!
 * In io.c, we implement alternatives with getwd() and the pwd
 * command!  Win32 has a getcwd with different signature!
 */
# ifndef WIN32
extern char	*getcwd proto((char *, size_t));
# endif

# ifdef HAS_SYMLINKS
/* bufsiz might be of type int in old systems */
extern int readlink proto((const char */*path*/, char */*buf*/, size_t /*bufsiz*/));
# endif

extern int	access proto((const char */*path*/, int /*mode*/));
# ifndef F_OK	/* in POSIX <unistd.h> */
#  define F_OK	0	/* does file exist? */
#  define X_OK	1	/* is it executable by caller? */
#  define W_OK	2	/* writable by caller? */
#  define R_OK	4	/* readable by caller? */
# endif

extern int	creat proto((const char */*path*/, jmode_t /*mode*/));
	/* open may have an optional third argument, promo(jmode_t) mode */
extern int	open proto((const char */*path*/, int /*flags*/, ...));

extern jmode_t umask proto((jmode_t));
# ifdef NO_MKSTEMP
#  ifndef NO_MKTEMP
extern char *mktemp proto((char */*template*/));
#  endif
# else
extern int mkstemp proto((char */*template*/));
# endif

# ifdef MSC51
extern SSIZE_T	read proto((int /*fd*/, char * /*buf*/, size_t /*nbytes*/));
extern SSIZE_T	write proto((int /*fd*/, const char * /*buf*/, size_t /*nbytes*/));
# else
extern SSIZE_T	read proto((int /*fd*/, UnivPtr /*buf*/, size_t /*nbytes*/));
extern SSIZE_T	write proto((int /*fd*/, UnivConstPtr /*buf*/, size_t /*nbytes*/));
# endif

# if !defined(WATCOMC) && !defined(ZTCDOS) && !defined(__BORLANDC__) && !defined(_MSC_VER)
/* Zortech incorrectly defines argv as const char **.
 * Borland incorrectly defines argv as char *[] and omits some consts
 * Watcom incorrectly defines argv as const char *const
 * MSC incorrectly defines argv as char const* const *
 * on some or all exec*() prototype declarations that they supply.
 * Jove uses spawn on these platforms anyway.
 */
extern int	execl proto((const char * /*path*/, const char * /*arg*/, ...));
extern int	execlp proto((const char * /*file*/, const char * /*arg*/, ...));
extern int	execv proto((const char * /*path*/, char *const /*argv*/[]));
extern int	execve proto((const char * /*path*/, char *const /*argv*/[],
			      char *const /*envp*/[]));
extern int	execvp proto((const char * /*file*/, char *const /*argv*/[]));
# endif

# ifdef MSC51
#  undef const
# endif

extern void	_exit proto((int));	/* exit(), without flush, etc. */

extern unsigned	alarm proto((unsigned /*seconds*/));

extern int	pipe proto((int *));
extern int	close proto((int));
extern int	dup proto((int));
extern int	dup2 proto((int /*old_fd*/, int /*new_fd*/));
extern off_t	lseek proto((int /*fd*/, off_t /*offset*/, int /*whence*/));
extern int	fchmod proto((int /*fd*/, jmode_t /*mode*/));
extern int	chown proto((const char *, int, int));

extern int	unlink proto((const char */*path*/));

#endif /* !POSIX_UNISTD */

#if defined(IBMPCDOS) || defined(WIN32)
  /*
   * At least as of 2007, perhaps earlier, the Microsoft runtime has
   * no X_OK and considers a value of 1 to be invalid, e.g see
   * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=30972
   * https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/access-waccess?view=vs-2019
   * Older runtimes may have accepted it, but VS 2019, in Win 7 Pro, 
   * crashes the program.
   */
# define J_X_OK F_OK
#else
# define J_X_OK X_OK
#endif

#ifndef FULL_UNISTD

extern int	fsync proto((int));

/* BSD UNIX
 *
 * Note: in most systems, declaration of a non-existant function is
 * OK if the function is never actually called.  The parentheses around
 * the name prevent any macro expansion.  Of course, if the types in the
 * prototype are not declared, the compiler gets upset.
 */

# ifdef USE_BCOPY
extern void	UNMACRO(bcopy) proto((UnivConstPtr, UnivPtr, size_t));
extern void	UNMACRO(bzero) proto((UnivPtr, size_t));
# endif

#endif /* !FULL_UNISTD */

/* termcap */
#ifdef TERMCAP
# ifdef JTC
#define DEFINE_PC_BC_UP_OSPEED 1 /* curses declares these, jtc does not! should it?*/
extern char *	UNMACRO(jtcarg1) proto((const char *, int /* parm */));
extern char *	UNMACRO(jtcarg2) proto((const char *, int /*destcol*/, int /*destline*/));
#  define	targ1(s, i)	jtcarg1(s, i)
#  define	targ2(s, c, l)	jtcarg2(s, c, l)
# else /* !JTC */
#  ifdef TERMINFO
extern char	*UNMACRO(tparm) proto((const char *, ...));
#   define	targ1(s, i)	tparm(s, i)
#   define	targ2(s, c, l)	tparm(s, l, c)
#  else /* !TERMINFO */
extern char	*UNMACRO(tgoto) proto((const char *, int /*destcol*/, int /*destline*/));
#   define	targ1(s, i)	tgoto(s, 0, i)
#   define	targ2(s, c, l)	tgoto(s, c, l)
#  endif /* TERMINFO */
# endif /* JTC */
#endif /* TERMCAP */
