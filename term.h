/************************************************************************
 * This program is Copyright (C) 1986-1994 by Jonathan Payne.  JOVE is  *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

/* note: most exports are declared in termcap.h */

extern bool	CanScroll;	/* can this terminal scroll? */

extern void
	getTERM proto((void)),
	putpad proto((const char *str, int lines)),
	putmulti proto((const char *ss, const char *ms, int num, int lines)),
	settout proto((void));
