/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

extern int
	alphacomp proto((char **a, char **b)),
	scandir proto((char *dir, char ***nmptr, int (*qualify)(), int (*sorter)()));

extern void
	freedir proto((char * * *nmptr,int nentries));

