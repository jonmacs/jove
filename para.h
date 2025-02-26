/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern void
	DoJustify proto((LinePtr l1, int c1,
		LinePtr l2, int c2, jbool scrunch, int indent)),
	do_rfill proto((jbool ulm));

/* Commands: */

extern void
	BackPara proto((void)),
	ForPara proto((void)),
	FillParagraph proto((void)),
	FillRegion proto((void));

/* Variables: */

extern jbool
	SpaceSent2;		/* VAR: space-sentence-2 */

extern int
	LMargin,		/* VAR: left margin */
	RMargin;		/* VAR: right margin */

extern char
	ParaDelim[128];	/* VAR: paragraph-delimiter-pattern */
