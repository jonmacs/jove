/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

extern int
	jscandir proto((const char *dir, char ***nmptr,
		bool (*qualify) ptrproto((char *)),
		int (*sorter) ptrproto((UnivConstPtr, UnivConstPtr))));

extern void
	freedir proto((char ***nmptr,int nentries));

#ifdef MSFILESYSTEM
/* NOTE: MatchDir affects any call to jscandir */
extern bool	MatchDir;
#endif
