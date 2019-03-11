/************************************************************************
 * This program is Copyright (C) 1986-1994 by Jonathan Payne.  JOVE is  *
 * provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is *
 * included in all the files.                                           *
 ************************************************************************/

#ifdef IBMPC	/* the body is the rest of this file */

typedef unsigned char	BYTE;
typedef unsigned int	WORD;

extern void
	pcSetTerm proto((void)),
	pcUnsetTerm proto((void)),
	dobell proto((int n)),
	scr_win proto((int, BYTE, BYTE, BYTE, BYTE)),
	clr_page proto((void)),
	clr_eoln proto((void));

/* Variables: */

extern int
	Txattr,	/* VAR: text-attribute (white on black) */
	Mlattr,	/* VAR: mode-line-attribute (black on white) */
	Hlattr;	/* VAR: highlight-attribute */

#endif /* IBMPC */
