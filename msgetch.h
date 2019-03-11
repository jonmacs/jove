/************************************************************************
 * This program is Copyright (C) 1986-1994 by Jonathan Payne.  JOVE is  *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

/* MSDOS keyboard routines */

#ifdef MSDOS	/* the body is the rest of this file */

# ifdef IBMPC
#  define	PCNONASCII	0xFF	/* prefix for peculiar IBM PC key codes */
extern bool enhanced_keybrd;	/* VAR: exploit "enhanced" keyboard? */
# endif

extern ZXchar	getrawinchar proto((void));
extern bool	rawkey_ready proto((void));

#endif /* MSDOS */
