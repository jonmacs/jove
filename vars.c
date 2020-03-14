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

private int
varcmp(p1, p2)
UnivConstPtr	p1, p2;
{
	const struct variable *v1 = (const struct variable *) p1;
	const struct variable *v2 = (const struct variable *) p2;
	return strncmp(v1->Name, v2->Name, strlen(v1->Name));
}

const char *
getvar(name, vbuf, vbufsize)
const char	*name;
char		*vbuf;
size_t		vbufsize;
{
	struct variable		vkey;
	const struct variable	*vp;
	vkey.Name = name;
	vp = (const struct variable *) bsearch((UnivConstPtr)&vkey,
		(UnivConstPtr)variables, elemsof(variables) - 1,/* ignore NULL */
		sizeof(struct variable), varcmp);
	if (vp == NULL)
		return NULL;
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
