/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

extern void
	DoJustify proto((struct line *l1, int c1,
		struct line *l2, int c2, bool scrunch, int indent)),
	do_rfill proto((bool ulm));

/* Commands: */

extern void
	BackPara proto((void)),
	ForPara proto((void)),
	Justify proto((void)),
	RegJustify proto((void));

/* Variables: */
extern int
	LMargin,		/* left margin */
	RMargin;		/* right margin */
