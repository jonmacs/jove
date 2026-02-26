/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern int
	jscandir(const char *dir, char ***nmptr,
		jbool (*qualify)(const char *),
		int (*sorter)(const void *, const void *));

extern void
	freedir(char ***nmptr, int nentries);

#ifdef MSFILESYSTEM
/* NOTE: MatchDir affects any call to jscandir */
extern jbool	MatchDir;
#endif
