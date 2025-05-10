/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* As of 5.x, Jove requires C89 and POSIX.1-conforming prototyped headers */

#ifdef USE_MEMORY_H
# include <memory.h>
#endif

#ifdef EWOULDBLOCK
# define RETRY_ERRNO(e) ((e) == EINTR || (e) == EAGAIN || (e) == EWOULDBLOCK)
#else
/* older Unix, e.g. V7 */
# define RETRY_ERRNO(e)	((e) == EINTR || (e) == EAGAIN)
#endif

#ifdef MSFILESYSTEM
# include <io.h>
#else
# include <unistd.h>
# include <fcntl.h>
#endif

#ifndef F_OK	/* Usually in POSIX <unistd.h> */
# define F_OK	0	/* does file exist? */
# define X_OK	1	/* is it executable by caller? */
# define W_OK	2	/* writable by caller? */
# define R_OK	4	/* readable by caller? */
#endif

#ifndef O_RDONLY	/* Usually in POSIX <fcntl.h> or Unix <sys/file.h> */
# define O_RDONLY	0
# define O_WRONLY	1
# define O_RDWR		2
#endif

#if defined(WIN32)
# define JSSIZE_T  int
# define JRWSIZE_T unsigned int /* MSVC for 64bit uses this, not size_t */
#else
# define JSSIZE_T  ssize_t
# define JRWSIZE_T size_t
#endif

#ifndef O_BINARY	/* needed for MSDOS and perhaps others */
# define O_BINARY	0	/* fake if missing with value harmless when ORed */
#endif
#ifndef O_CLOEXEC
# define O_CLOEXEC	0
#endif

extern void	_exit(int);	/* exit(), without flush, etc. */

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

/* termcap */
#ifdef TERMCAP
# ifdef JTC
#define DEFINE_PC_BC_UP_OSPEED 1 /* curses declares these, jtc does not! should it?*/
extern char	*jtcarg1(const char *, int /* parm */);
extern char	*jtcarg2(const char *, int /*destcol*/, int /*destline*/);
#  define	targ1(s, i)	jtcarg1(s, i)
#  define	targ2(s, c, l)	jtcarg2(s, c, l)
# else /* !JTC */
#  ifdef TERMINFO
extern char	*tparm(const char *, ...);  /* SysV/POSIX curses.h or term.h */
#   define	targ1(s, i)	tparm(s, i)
#   define	targ2(s, c, l)	tparm(s, l, c)
#  else /* !TERMINFO */
extern char	*tgoto(const char *, int /*destcol*/, int /*destline*/); /* BSD termcap */
#   define	targ1(s, i)	tgoto(s, 0, i)
#   define	targ2(s, c, l)	tgoto(s, c, l)
#  endif /* TERMINFO */
# endif /* JTC */
#endif /* TERMCAP */

/* originally sysprocs.h, simpler to just incorporate it here */

/* The diversity of process management is complicated and difficult to handle.
 * - In some systems (noteably POSIX), a process id has type "pid_t"
 * - V7 only has wait.  POSIX has waitpid (with options).  BSD has
 *   wait3.  Someone has wait2.
 * - The status result set by wait and used by WIF* has type
 *   "union wait" in BSD, but "int" everywhere else.
 * - The WIF* functions are defined in <sys/wait.h> by BSD and POSIX.
 * - WTERMSIG seems to be a creation of POSIX
 * - Some systems have vfork(1) and perform better if it is used
 *   in place of fork(1).
 *
 * This header attempts to span this diversity.  We provide:
 * - POSIX pid_t
 * - wait_opt, to accept options (and use them, if possible).
 * - wait_status_t
 * - WIF*
 * - WTERMSIG
 */

#ifdef MSDOS_PROCS
# include <process.h>
#endif

#ifdef POSIX_PROCS
# include <sys/wait.h>
  typedef int	wait_status_t;
# define wait_opt(stat_loc, options)	waitpid(-1, stat_loc, options)

#else /*!POSIX_PROCS*/

# ifndef _PID_T_ /* MINGW, maybe WIN32 has sys/types.h with pid_t, it seems */
   typedef int	pid_t;
# endif

# ifdef BSD_WAIT
#  include <sys/wait.h>
  typedef union wait	wait_status_t;

#  ifndef WEXITSTATUS
#   define WEXITSTATUS(w)	((w).w_retcode)
#  endif

#  ifndef WTERMSIG
#   define WTERMSIG(w)	((w).w_termsig)
#  endif

#  ifndef WAIT3
#   define wait_opt(stat_loc, options)	wait2(stat_loc, options)
#  else
#   define wait_opt(stat_loc, options)	wait3(stat_loc, options, (struct rusage *)NULL)
#  endif

# else /*!BSD_WAIT*/

  typedef int	wait_status_t;

#  ifdef UNIX

#   ifndef WIFSTOPPED
#    define WIFSTOPPED(w)	((w & 0377) == 0177)
#    define WIFEXITED(w)	((w & 0377) == 0)
#    define WIFSIGNALED(w)	(((w >> 8) & 0377) == 0)
#    define WEXITSTATUS(w)	((w >> 8) & 0377)
#    define WTERMSIG(w)	(w & 0177)
#   endif

#   define wait_opt(stat_loc, options)		wait(stat_loc)
    /* should be correct if all preceding typedefs or includes worked out */
    extern pid_t wait(wait_status_t *);

#  endif /* UNIX */

# endif /*!BSD_WAIT*/
#endif /*!POSIX_PROCS*/

/* This nest of #ifdefs is simply to define NEWPG() which makes
 * the current process a process group leader of a new process group.
 * ??? pid_t may be changed by default argument promotions.
 * If so, this prototype might be wrong.
 */
#ifdef POSIX_PROCS
# define NEWPG()	setpgid(0, getpid())
#else /* !POSIX_PROCS */
# ifdef BSD_SETPGRP
#  define NEWPG()	setpgrp(0, getpid())
# else /* !(defined(BSD_SETPGRP) || defined(POSIX_PROCS)) */
#  define NEWPG()	setpgrp()
# endif /* !(defined(BSD_SETPGRP) || defined(POSIX_PROCS)) */
#endif /* !POSIX_PROCS */

