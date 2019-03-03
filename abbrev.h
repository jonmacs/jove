/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#ifdef ABBREV	/* the body is the rest of this file */

extern void	AbbrevExpand proto((void));

/* Variables: */

extern bool	AutoCaseAbbrev;		/* VAR: automatically do case on abbreviations */

/* Commands: */
extern void
	BindMtoW proto((void)),
	DefMAbbrev proto((void)),
	DefGAbbrev proto((void)),
	SaveAbbrevs proto((void)),
	RestAbbrevs proto((void)),
	EditAbbrevs proto((void));

#endif /* ABBREV */
