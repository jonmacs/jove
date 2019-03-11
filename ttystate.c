/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#include "jove.h"
#include "ttystate.h"

#ifdef	UNIX

#ifdef TERMIO
struct termio	sg[2];
#endif
#ifdef TERMIOS
struct termios	sg[2];
#endif

#ifdef SGTTY

struct sgttyb	sg[2];

# ifdef	TIOCSLTC
struct ltchars	ls[2];
# endif	/* TIOCSLTC */

# ifdef	TIOCGETC
struct tchars	tc[2];
# endif

# ifdef	PASS8			/* use pass8 instead of raw for meta-key */
int	lmword[2];		/* local mode word */
# endif

#endif	/* SGTTY */

#endif	/* UNIX */
