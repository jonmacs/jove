/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/* Tuning: configure JOVE to installer's taste */

#define TUNED		1	/* don't touch this */

#include "sysdep.h"

/*
 * Select or comment out as many of the following features of Jove as you'd
 * like.  In general, most of them are useful or pleasant, but many of them
 * are non-portable, or make Jove larger.
 *
 * By default, most are included if appropriate.  BAREBONES suppresses most
 * of them.  With BAREBONES defined, Jove should compile on a lot of systems.
 * It should be considered the starting point for any port of Jove to a new
 * system.  An advantage of BAREBONES is that it shrinks JOVE's already
 * modest size.
 */

#ifndef BAREBONES

# define ABBREV		1	/* word abbreviation mode */
# define BACKUPFILES	1	/* enable the backup files code */

# ifdef UNIX
#  define BIFF		1	/* if you have biff (or the equivalent) */
# endif

# define CMT_FMT		1	/* include the comment formatting routines */
# define F_COMPLETION	1	/* filename completion */

/* Include code to IDchar (optimize for terminals with character
 * insert/delete modes
 */
# ifdef TERMCAP
#  define ID_CHAR		1
# endif

# ifdef TERMCAP
#  define	HIGHLIGHTING	1	/* highlighting is used for mark and scrollbar */
# endif

# if !defined(NO_IPROCS) && (defined(PIPEPROCS) || defined(PTYPROCS))
#  define IPROCS		1	/* interactive processes */
# endif

# define LISP		1	/* include the code for Lisp Mode */

# ifndef MAC
#  define	SUBSHELL	1	/* enable various uses of subshells */
# endif

# if defined(SUBSHELL) && !defined(SPELL)
#  define SPELL		"spell %s"	/* spell words and buffer commands (requires SUBSHELL) */
# endif

# define RECOVER	1	/* include code to attempt recovery from JOVE crashes */

#endif /* !BAREBONES */

#ifndef IPROCS
# undef PIPEPROCS	/* defined only if IPROCS is */
# undef PTYPROCS	/* defined only if IPROCS is */
#endif

#ifndef MSDOS
# define DFLT_MODE	0666	/* file will be created with this mode */
#endif

/* If the compiler does not support void, use -Dvoid=int or
 * typedef int	void;
 */

/*
 * USE_PROTOTYPE must be defined for compilers that support prototypes but are
 * NOT ANSI C, i.e. do not have __STDC__ == 1.  USE_PROTOTYPE will be
 * automatically defined for ANSI compilers.
 */
/* #define USE_PROTOTYPES	1 */
