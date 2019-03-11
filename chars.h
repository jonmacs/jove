/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#define CTL(c)		((c) & 037)
#define METABIT		0200
#define DEL		'\177'
#define LF		CTL('J')
#define CR		CTL('M')
#define BS		CTL('H')
#define ESC		'\033'
