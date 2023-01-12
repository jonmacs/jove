/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#include "jove.h"
#include "extend.h"
#include "vars.h"

/* included for variable declarations */
#include "abbrev.h"
#include "ask.h"
#include "c.h"
#include "jctype.h"
#include "disp.h"
#include "insert.h"
/* #include "io.h" */
#include "sysprocs.h"	/* needed for iproc.h */
#include "iproc.h"
/* #include "jove.h" */
#include "mac.h"
#include "mouse.h"
#include "para.h"
#include "proc.h"
#include "re.h"
#include "reapp.h"
#include "rec.h"
#include "screen.h"
#include "term.h"
#include "ttystate.h"
/* #include "util.h" */
#include "wind.h"


#define VAR(v)	(UnivPtr)(v), sizeof(v)
#include "vars.tab"

const char *
getvar(name, vbuf, vbufsize)
const char	*name;
char		*vbuf;
size_t		vbufsize;
{
	register ZXchar	c;
	const struct variable	*vp, *which = NULL;
	size_t	vlen = strlen(name);
	int ic;

	/* look it up (in the reduced search space) */
	c = ZXC(name[0]);
	ic = IDX(c);
	if (ic >= 0 && ic <= IDXSZ
	    && (vp = varidx[ic]) != NULL) {
		for (; vp->Name != NULL && vp->Name[0] == name[0]; vp++) {
			if (strncmp(vp->Name, name, vlen) == 0) {
				if (vp->Name[vlen] != '\0' &&
				    which != NULL) {
					complain("[\"%s\" ambiguous]", name);
					/* NOTREACHED */
				}
				which = vp;
				if (vp->Name[vlen] == '0')
				    break;
			}
		}
	}
	if (which == NULL) {
		complain("[variable \"%s\" unknown]", name);
			/* NOTREACHED */
	}
	vpr_aux(vp, vbuf, vbufsize);
	return vbuf;
}

const data_obj *
findvar(prompt)
const char	*prompt;
{
	static const char	*strings[elemsof(variables)];
	static int	last = -1;

	if (strings[0] == NULL) {
		register const char	**strs = strings;
		register const struct variable	*v = variables;

		for (; v->Name; v++)
			*strs++ = v->Name;
		*strs = NULL;
	}

	last = complete(strings, last >= 0? strings[last] : (char *)NULL,
		prompt, ALLOW_OLD);
	return (data_obj *) &variables[last];
}
