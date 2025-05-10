/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#ifdef UNIX

/* Various tty state structures.
 * Each is an array, subscripted by one of "NO" or "YES".
 * YES is set suitably for JOVE.
 * NO is reset back to initial state.
 */

#include <termios.h>
extern struct termios	sg[2];

  /* JOVE needs to know about tab expansion to infer whether it should
   * use tabs to optimize output (jove.c).
   * It also must be able to turn tab expansion off for a pty (iproc.c).
   * The "normal" way of signifying tab expansion in a termio or termios
   * struct is to set the bits in c_oflag masked by TABDLY to TAB3.
   * Instead, some systems (eg. 386BSD) have a single bit named OXTABS,
   * from which we synthesize fake definitions of TABDLY and TAB3.
   * Some other systems have no bit, so we must presume that tabs don't
   * work and that we needn't change the setting.
   */
# ifndef TABDLY
#  ifdef OXTABS
#   define TABDLY	OXTABS
#   define TAB3	TABDLY
#  else
#   define TABDLY	0	/* tab expansion is unknown and uncontrollable */
#  endif
# endif

# ifdef USE_BSDTTYINCLUDE
#  include <bsdtty.h>
#  ifdef TIOCSLTC
#   define USE_TIOCSLTC	1
#  endif
# endif

# ifdef USE_TIOCSLTC
extern struct ltchars	ls[2];
# endif /* USE_TIOCSLTC */

 /* to get a definition for TIOCGWINSZ, we include <sys/ioctl.h> */
# include <sys/ioctl.h>

/* Variables: */

extern jbool	OKXonXoff;		/* VAR: XON/XOFF can be used as ordinary chars */
extern ZXchar	IntChar;		/* VAR: ttysetattr sets this to generate QUIT */

#endif /* UNIX */

#ifdef BIFF
extern jbool	DisBiff;		/* VAR: turn off/on biff with entering/exiting jove */
#endif
