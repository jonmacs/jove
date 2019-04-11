/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#include "jove.h"
#include "term.h"

#include "fp.h"

/* universal termcap-like definitions (declared in jove.h) */

int
	SG = 0,		/* number of magic cookies left by SO and SE */
	LI,		/* number of lines */
	ILI,		/* number of internal lines (LI - 1) */
	CO;		/* number of columns (CO <= MAXCOLS) */

bool
	TABS = NO;		/* terminal supports tabs */

#ifndef UNIX
void
settout()
{
# ifndef NO_JSTDOUT
	flushscreen();		/* flush the one character buffer */
	ScrBufSize = min(MAXTTYBUF, 256);
	jstdout = fd_open("/dev/tty", F_WRITE|F_LOCKED, 1, (char *)NULL, ScrBufSize);
# endif
}
#endif /* !UNIX */
