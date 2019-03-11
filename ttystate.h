/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/* Various tty state structures.
 * Each is an array, subscripted by one of "NO" or "YES".
 * YES is set suitably for JOVE.
 * NO is reset back to initial state.
 */

#ifdef	UNIX

#ifdef SGTTY
# include <sgtty.h>
extern struct sgttyb	sg[2];
#endif

#ifdef TERMIO
# include <sys/ioctl.h>
# include <termio.h>
extern struct termio	sg[2];
#endif

#ifdef TERMIOS
# include <sys/ioctl.h>
# include <termios.h>
extern struct termios	sg[2];
# ifndef VDSUSP
#  define VDSUSP	VSUSP	/* non-Posixism in Irix3.3, may exist in others */
# endif
#endif

#ifdef	SGTTY

# ifdef	TIOCSLTC
extern struct ltchars	ls[2];
# endif	/* TIOCSLTC */

# ifdef	TIOCGETC
extern struct tchars	tc[2];
# endif

# ifdef	PASS8			/* use pass8 instead of raw for meta-key */
extern int	lmword[2];		/* local mode word */
# endif

#endif	/* SGTTY */

#endif	/* UNIX */
