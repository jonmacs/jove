/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#include "jove.h"
#include "ctype.h"
#include "extend.h"
#include "macros.h"

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
#include "paragraph.h"
#include "proc.h"
#include "reapp.h"
#include "wind.h"

#define	PROC(p)	(p)
#include "commands.tab"

data_obj	*LastCmd;
char	*ProcFmt = ": %f ";

data_obj *
findcom(prompt)
const char	*prompt;
{
	/* This is for faster startup.  This just reads until a space or a
	   tab or a newline character is reached, and then does a
	   semi-hashed lookup on that string.  This should be much faster
	   than initializing the minibuffer for each line. */
	if (InJoverc) {
		char	cmdbuf[128];
		register const struct cmd	*cmd;
		register char	*cp = cmdbuf;
		register int	c;
		const struct cmd	*which = NULL;
		size_t	cmdlen;
		static const struct cmd	*cmdhash[26];
		static bool	beenhere = NO;

/* special case for prefix commands--only upper case ones */
#define hash(c)	((c) - 'a')

		/* initialize the hash table */
		if (!beenhere) {
			char	lastc = '\0';

			for (cmd = commands; cmd->Name != NULL; cmd++) {
				if (lastc != cmd->Name[0]) {
					lastc = cmd->Name[0];
					cmdhash[hash(lastc)] = cmd;
				}
			}
			beenhere = YES;
		}
#ifdef	MAC
		/* ??? Is this necessary?  The input is comming from a file! */
		menus_off();	/* Block menu choices during input */
#endif
		/* gather the cmd name */
		while (((c = getch()) != EOF) && !strchr(" \t\r\n", c)) {
			if (jisupper(c))
				c = jtolower(c);
			*cp++ = c;
		}
		if (c == EOF)
			return NULL;
		*cp = '\0';
		cmdlen = cp - cmdbuf;
		if (cmdlen == 0)
			return NULL;

		/* look it up (in the reduced search space) */
		if (jislower(cmdbuf[0])
		&& (cmd = cmdhash[hash(cmdbuf[0])]) != NULL) {
		    for (; cmd->Name != NULL && cmd->Name[0] == cmdbuf[0]; cmd++) {
			if (strncmp(cmd->Name, cmdbuf, cmdlen) == 0) {
				if (cmd->Name[cmdlen] == '\0')
					return (data_obj *) cmd;
				if (which != NULL)
					complain("[\"%s\" ambiguous]", cmdbuf);
				which = cmd;
			}
		    }
		}
		if (which == NULL)
			complain("[\"%s\" unknown]", cmdbuf);
		return (data_obj *) which;
	} else {
		static char	*strings[(sizeof commands) / sizeof (commands[0])];
		static bool	beenhere = NO;
		register int	com;

		if (!beenhere) {
			register char	**strs = strings;
			register const struct cmd	*c = commands;

			do ; while ((*strs++ = (*c++).Name) != NULL);
			beenhere = YES;
		}

		if ((com = complete(strings, prompt, CASEIND)) < 0)
			return NULL;
		return (data_obj *) &commands[com];
	}
#undef	hash
}

const struct cmd *
FindCmd(proc)
register void	(*proc) proto((void));
{
	register const struct cmd	*cp;

	for (cp = commands; cp->Name; cp++)
		if (cp->c_proc == proc)
			return cp;
	return NULL;
}

void
ExecCmd(cp)
register data_obj	*cp;
{
	LastCmd = cp;
	if (cp->Type & MAJOR_MODE) {
		SetMajor((cp->Type >> 8));
	} else if (cp->Type & MINOR_MODE) {
		TogMinor((cp->Type >> 8));
	} else	switch (cp->Type&TYPEMASK) {
		case MACRO:
			do_macro((struct macro *) cp);
			break;

		case FUNCTION:
		    {
			register struct cmd	*cmd = (struct cmd *) cp;

			if (cmd->c_proc) {
				if ((cmd->Type & MODIFIER)
				&& BufMinorMode(curbuf, ReadOnly))
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
