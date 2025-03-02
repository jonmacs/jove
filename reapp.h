/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern void
	find_tag(char *tag, jbool localp);

/* Commands: */

extern void
	FDotTag(void),
	FindTag(void),
	IncFSearch(void),
	IncRSearch(void),
	QRepSearch(void),
	RegReplace(void),
	RepSearch(void),
	FSrchND(void),
	ForSearch(void),
	RSrchND(void),
	RevSearch(void);

/* Variables: */

extern ZXchar	SExitChar;		/* VAR: type this to stop i-search */
extern char	TagFile[FILESIZE];	/* VAR: default tag file */
extern jbool	UseRE;			/* VAR: use regular expressions in search */
