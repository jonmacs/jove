/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/* MSDOS keyboard routines */

#ifdef	MSDOS

#ifdef	IBMPC
extern int specialkey;
#endif

extern int	getrawinchar proto((void));
extern bool	rawkey_ready proto((void));

#endif	/* MSDOS */
