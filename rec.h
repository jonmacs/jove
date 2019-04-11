/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#ifdef RECOVER	/* the body is the rest of this file */

extern void
	SyncRec proto((void)),
	rectmpname proto((char *)),
	recclose proto((void)),
	recremove proto((void));

extern int	ModCount;	/* number of buffer mods since last sync */

/* Variables: */

extern int	SyncFreq;	/* VAR: how often to sync the file pointers */

#endif /* RECOVER */
