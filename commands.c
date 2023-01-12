/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#include "jove.h"
#include "jctype.h"
#include "extend.h"
#include "macros.h"
#include "mouse.h"

/* included for command routine declarationss */
#include "abbrev.h"
/* #include "argcount.h */
/* #include "buf.h" */
#include "c.h"
#include "case.h"
#include "commands.h"
#include "delete.h"
#include "disp.h"
/* #include "extend.h" */
#include "insert.h"
/* #include "io.h" */
#include "sysprocs.h"	/* needed for iproc.h */
#include "iproc.h"
/* #include "jove.h" */
/* #include "keymaps.h" */
/* #include "macros.h" */
#include "marks.h"
#include "misc.h"
#include "move.h"
#include "para.h"
#include "proc.h"
#include "reapp.h"
#include "wind.h"

#define	PROC(p)	(p)
#include "commands.tab"

const data_obj	*LastCmd;
const char	*ProcFmt = ": %f ";

const data_obj *
findcom(prompt)
const char	*prompt;
{
	if (InJoverc) {
		/* For faster rc file processing on startup, read until a space
		 * or a tab or a newline character is reached, and then do a
		 * semi-indexedlookup on that string.  This should be much faster
		 * than minibuffer completion for each line.
		 */
		char	cmdbuf[128];
		register const struct cmd	*cmd;
		register char	*cp = cmdbuf;
		register ZXchar	c;
		const struct cmd	*which = NULL;
		size_t	cmdlen;
		int ic;

#ifdef MAC
		/* ??? Is this necessary?  The input is comming from a file! */
		menus_off();	/* Block menu choices during input */
#endif
		/* gather the cmd name */
		while (jisprint(c = getch()) && c != ' ') {
			*cp++ = CharDowncase(c);
			if (cp == &cmdbuf[sizeof(cmdbuf)]) {
				complain("command too long");
				/* NOTREACHED */
			}
		}
		*cp = '\0';
		cmdlen = cp - cmdbuf;

		/* look it up (in the reduced search space) */
		c = ZXC(cmdbuf[0]);
		ic = IDX(c);
		if (ic >= 0 && ic <= IDXSZ
		    && (cmd = cmdidx[ic]) != NULL) {
			for (; cmd->Name != NULL && cmd->Name[0] == cmdbuf[0]; cmd++) {
				if (strncmp(cmd->Name, cmdbuf, cmdlen) == 0) {
					if (cmd->Name[cmdlen] == '\0')
						return (data_obj *) cmd;

					if (which != NULL) {
						complain("[cmd \"%s\" ambiguous]", cmdbuf);
						/* NOTREACHED */
					}
					which = cmd;
				}
			}
		}
		if (which == NULL) {
			complain("[\"%s\" unknown]", cmdbuf);
			/* NOTREACHED */
		}
		return (data_obj *) which;
	} else {
		static const char	*strings[elemsof(commands)];
		static int	last = -1;

		if (strings[0] == NULL) {
			register const char	**strs = strings;
			register const struct cmd	*c = commands;

			do {} while ((*strs++ = (*c++).Name) != NULL);
		}
		last = complete(strings, last >= 0? strings[last] : (char *)NULL,
			prompt, CASEIND | ALLOW_OLD);
		return (data_obj *) &commands[last];
	}
}

const struct cmd *
FindCmd(proc)
register cmdproc_t proc;
{
	register const struct cmd	*cp;

	for (cp = commands; cp->Name; cp++)
		if (cp->c_proc == proc)
			return cp;
	return NULL;
}

void
ExecCmd(cp)
register const data_obj	*cp;
{
	LastCmd = cp;
	if (cp->Type & MAJOR_MODE) {
		SetMajor((cp->Type >> MAJOR_SHIFT));
	} else if (cp->Type & MINOR_MODE) {
		TogMinor((cp->Type >> MAJOR_SHIFT));
	} else	switch (cp->Type&TYPEMASK) {
		case MACRO:
			do_macro((struct macro *) cp);
			break;

		case COMMAND:
		    {
			register struct cmd	*cmd = (struct cmd *) cp;

			if (cmd->c_proc != NULL) {
				if ((cmd->Type & MODIFIER)
				&& BufMinorMode(curbuf, BReadOnly))
				{
					rbell();
					message("[Buffer is read-only]");
				} else {
					(*cmd->c_proc)();
				}
			}
		    }
			break;
	}
}
