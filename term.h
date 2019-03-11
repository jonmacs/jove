/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

/* note: most exports are declared in termcap.h */

extern bool	CanScroll;	/* can this terminal scroll? */

#ifndef	IBMPC
extern void	getTERM proto((void));
#endif

extern void
	putpad proto((char *str, int lines)),
	putmulti proto((char *ss, char *ms, int num, int lines)),
	settout proto((char *ttbuf));
