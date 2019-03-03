/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern void
	find_tag proto((char *tag,bool localp));

/* Commands: */

extern void
	FDotTag proto((void)),
	FindTag proto((void)),
	IncFSearch proto((void)),
	IncRSearch proto((void)),
	QRepSearch proto((void)),
	RegReplace proto((void)),
	RepSearch proto((void)),
	FSrchND proto((void)),
	ForSearch proto((void)),
	RSrchND proto((void)),
	RevSearch proto((void));

/* Variables: */

extern ZXchar	SExitChar;		/* VAR: type this to stop i-search */
extern char	TagFile[FILESIZE];	/* VAR: default tag file */
extern bool	UseRE;			/* VAR: use regular expressions in search */
