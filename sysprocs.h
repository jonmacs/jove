/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

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

# include <sys/types.h>	/* defines pid_t */
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
    extern pid_t wait proto((wait_status_t *));

#  endif /* UNIX */

# endif /*!BSD_WAIT*/
#endif /*!POSIX_PROCS*/

#ifndef FULL_UNISTD
# ifndef POSIX_UNISTD

/* ??? pid_t may be changed by default argument promotions.
 * If so, this prototype might be wrong.
 */
extern int	kill proto((pid_t /*pid*/, int /*sig*/));	/* signal.h */

extern pid_t	fork proto((void));
extern pid_t	getpid proto((void));
extern int	getuid proto((void));
extern int	setuid proto((int));
# endif /* !POSIX_UNISTD */

# ifdef USE_VFORK
extern int	UNMACRO(vfork) proto((void));
# endif
#endif /* !FULL_UNISTD */

/* This nest of #ifdefs is simply to define NEWPG() which makes
 * the current process a process group leader of a new process group.
 * ??? pid_t may be changed by default argument promotions.
 * If so, this prototype might be wrong.
 */
#ifdef POSIX_PROCS
# ifndef FULL_UNISTD
   extern int	UNMACRO(setpgid) proto((pid_t /*pid*/, pid_t /*pgid*/));
   extern pid_t UNMACRO(setsid) proto((void));
# endif
# define NEWPG()	setpgid(0, getpid())
#else /* !POSIX_PROCS */
# ifdef BSD_SETPGRP
#  ifndef FULL_UNISTD
   extern int	UNMACRO(setpgrp) proto((pid_t /*pid*/, pid_t /*pgrp*/));
#  endif
#  define NEWPG()	setpgrp(0, getpid())
# else /* !(defined(BSD_SETPGRP) || defined(POSIX_PROCS)) */
#  ifndef FULL_UNISTD
   extern int	UNMACRO(setpgrp) proto((void));
#  endif
#  define NEWPG()	setpgrp()
# endif /* !(defined(BSD_SETPGRP) || defined(POSIX_PROCS)) */
#endif /* !POSIX_PROCS */
