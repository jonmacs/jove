/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#include "jove.h"
#include "extend.h"
#include "vars.h"

/* included for variable declarations */
#include "abbrev.h"
#include "ask.h"
#include "c.h"
#include "disp.h"
#include "insert.h"
/* #include "io.h" */
#include "sysprocs.h"	/* needed for iproc.h */
#include "iproc.h"
/* include "jove.h" */
#include "mac.h"
#include "marks.h"
#include "paragraph.h"
#include "pcscr.h"
#include "proc.h"
#include "re.h"
#include "reapp.h"
#include "rec.h"
#include "screen.h"
/* #include "util.h" */
#include "wind.h"

#define VAR(v)	(UnivPtr)(v), sizeof(v)
#include "vars.tab"

data_obj *
findvar(prompt)
const char	*prompt;
{
	static char	*strings[(sizeof variables) / sizeof (struct variable)];
	static bool	beenhere = NO;
	register int	com;

	if (!beenhere) {
		register char	**strs = strings;
		register const struct variable	*v = variables;

		beenhere = YES;
		for (; v->Name; v++)
			*strs++ = v->Name;
		*strs = NULL;
	}

	if ((com = complete(strings, prompt, NOTHING)) < 0)
		return NULL;
	return (data_obj *) &variables[com];
}
