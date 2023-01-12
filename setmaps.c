/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

/*
 * Setmaps runs at build time, which makes life hard for
 * cross-compilers:  we need the tune.h/sysdep.h definitions
 * for the target machine, i.e.  whatever SYSDEFS the makefile
 * provides, but those may not be appropriate for the host,
 * which, might be a very different compiler.
 */
 
#define USE_STDIO_H 1
#define NO_EXTERNS 1
#include "jove.h"
#include "chars.h"
#include "commands.h"
#include "vars.h"

extern void	exit proto((int));

#define LINESIZE	100	/* hope this is big enough */
#define STACKLIMIT	10	/* max conditional depth */

#define	PROC(p)	(cmdproc_t)0	/* discard function pointers */
#include "commands.tab"

#define VAR(v)	NULL, (size_t)0	/* discard variable pointers */
#include "vars.tab"

private int
matchcmd(choices, what)
register const struct cmd	choices[];
register char	*what;
{
	register int	i;

	for (i = 0; choices[i].Name != NULL; i++) {
		if (what[0] == choices[i].Name[0]
		&& strcmp(what, choices[i].Name) == 0)
			return i;
	}
	return -1;
}

#ifdef MAC
matchvar(choices, what)
register const struct variable choices[];
register char	*what;
{
	register int	len;
	int	i;

	len = strlen(what);
	for (i = 0; choices[i].Name != NULL; i++) {
		if (what[0] == choices[i].Name[0]
		&& strcmp(what, choices[i].Name) == 0)
			return i;
	}
	return -1;
}
#endif

private int
StartsWith(s, pre)
const char *s, *pre;
{
    return strncmp(s, pre, strlen(pre)) == 0;
}

private char *
PPkey(c)
int	c;
{
	static char	str[16];
	char	*cp = str;

	if (c & METABIT) {
		c &= ~METABIT;
		strcpy(cp, "M-");
		cp += 2;
	}
	if (c == ESC) {
		strcpy(cp, "ESC");
	} else if (c < ' ') {
		*cp++ = '^';
		*cp++ = c + '@';
		*cp = '\0';
	} else if (c == DEL) {
		strcpy(cp, "^?");
	} else {
		*cp++ = c;
		*cp = '\0';
	}
	return str;
}

private void
extract(into, from)
char	*into,
	*from;
{
	from += 2;	/* Past tab and first double quote. */
	while ((*into = *from++) != '"')
		into += 1;
	*into = '\0';
}


int
main()
{
	FILE
		*ifile,
		*of;
	char
		line[LINESIZE],
		comname[LINESIZE];
	int
		comnum,
		lino,
		ch;
	struct {
		int	first;
		int	last;
		char	condition[LINESIZE];
	}
		stackspace[STACKLIMIT],	/* first entry not used */
		*sp = stackspace;
#ifdef MAC
	char	*which;
	int	filecnt = 0;
	bool	inmenu = NO;
	struct fname {
		char	*in, *out;
	};
	static const struct fname	fnt[] = {
			{ "keys.txt", "keys.c" },
			{ "menumaps.txt", "menumaps.c" },
			{ NULL, NULL }
	};
	const struct fname *fnp;
#endif /* MAC */

	static int cmdidx[IDXSZ], varidx[IDXSZ];
	int lastidx = -1, ic;
	
	for (comnum = 0; commands[comnum+1].Name != NULL; comnum++) {
		ic = IDX(commands[comnum].Name[0]);
		if (ic < 0 || ic >= IDXSZ) {
			fprintf(stderr, "command %s idx %d is out of range, must be 0..%d\n",
				commands[comnum].Name, ic, IDXSZ);
			exit(1);
		}
		if (lastidx != ic) {
			cmdidx[ic] = comnum;
			lastidx = ic;
		}
		if (strcmp(commands[comnum].Name, commands[comnum+1].Name) >= 0) {
			fprintf(stderr, "command %s is out of order\n",
				commands[comnum].Name);
			exit(1);
		}
	}
	lastidx = -1;
	for (comnum = 0; variables[comnum+1].Name != NULL; comnum++) {
		ic = IDX(variables[comnum].Name[0]);
		if (ic < 0 || ic >= IDXSZ) {
			fprintf(stderr, "variable %s idx %d is out of range, must be 0..%d\n",
				variables[comnum].Name, ic, IDXSZ);
			exit(1);
		}
		if (lastidx != ic) {
			varidx[ic] = comnum;
			lastidx = ic;
		}
		if (strcmp(variables[comnum].Name, variables[comnum+1].Name) >= 0) {
			fprintf(stderr, "variable %s is out of order\n", variables[comnum].Name);
			exit(1);
		}
	}
#ifdef MAC
	/* don't know how to redirect, so we do tricks */
for (fnp = fnt; fnp->in != NULL; fnp++) {
	printf("setmaps <%s >%s\n", fnp->in, fnp->out);
	ifile = fopen(fnp->in, "r");
	if (ifile == NULL) {
		perror(fnp->in);
		exit(1);
	}
	of = fopen(fnp->out, "w");
	if (of == NULL) {
		perror(fnp->out);
		exit(1);
	}
#else /* !MAC */
	ifile = stdin;
	of = stdout;
	if (ifile == NULL || of == NULL) {
		fprintf(stderr, "Cannot read input or write output.\n");
		exit(1);
	}
#endif /* !MAC */
	lino = 0;
	ch = 0;
	for (;;) {
		if (fgets(line, (int)sizeof(line), ifile) == NULL) {
			if (sp != stackspace) {
				fprintf(stderr, "EOF inside #if\n");
				exit(1);
			}
			fclose(of);
			fclose(ifile);
			break;
		}
		lino += 1;
		if (StartsWith(line, "/* IDX_TAG")) {
			int i, idx;
			fputs("const struct cmd *cmdidx[IDXSZ] = {\n", of);
			for (i = 0; i < IDXSZ; i++) {
				if (i == 0 || cmdidx[i] != 0) {
					idx = cmdidx[i];
					fprintf(of, "\t&commands[%d], /* %s */\n",
						idx, commands[idx].Name);
				} else {
					fputs("\tNULL,\n", of);
				}
			}
			fputs("};\nconst struct variable *varidx[IDXSZ] = {\n", of);
			for (i = 0; i < IDXSZ; i++) {
				if (i == 0 || varidx[i] != 0) {
					idx = varidx[i];
					fprintf(of, "\t&variables[%d], /* %s */\n",
						idx, variables[idx].Name);
				} else {
					fputs("\tNULL,\n", of);
				}
			}
			fputs("};", of);
		} else if (StartsWith(line, "#if")) {
			sp += 1;
			if (sp == &stackspace[STACKLIMIT]) {
				fprintf(stderr,
					"conditionals nested too deeply at line %d\n",
					lino);
				exit(1);
			}
			sp->first = ch;
			sp->last = -1;
			strcpy(sp->condition, line);
			fputs(line, of);
		} else if (StartsWith(line, "#else")) {
			if (sp == stackspace || sp->last != -1) {
				fprintf(stderr, "ifdef/endif mismatch at line %d!\n",
					lino);
				exit(1);
			}
			sp->last = ch;
			ch = sp->first;
			fputs(line, of);
		} else if (StartsWith(line, "#endif")) {
			if (sp == stackspace) {
				fprintf(stderr, "ifdef/endif mismatch at line %d!\n",
					lino);
				exit(1);
			}
			if (sp->last != -1 && ch != sp->last) {
				fprintf(stderr,
					"warning: unbalanced number of entries in #if ending at line %d",
					lino);
			}
			sp -= 1;
			fputs(line, of);
#ifdef MAC
		} else if (StartsWith(line, "#MENU")) {
			inmenu = YES;
#endif
		} else if (StartsWith(line, "\t\"")) {
			extract(comname, line);
			if (strcmp(comname, "unbound") == 0) {
				comnum = -1;
			} else {
				comnum = matchcmd(commands, comname);
#ifdef MAC
				which = "commands";
				if (comnum < 0 && inmenu) {
					comnum = matchvar(variables, comname);
					which = "variables";
				}
#endif
				if (comnum < 0) {
					fprintf(stderr,
						"warning: cannot find \"%s\", line %d",
						comname, lino);
					if (sp == stackspace) {
						fprintf(stderr, ".\n");
					} else {
						/* Note: condition ends with \n */
						fprintf(stderr, ", inside%s %s",
							sp->last == -1? "" : " else of",
							sp->condition);
					}
				}
			}
#ifdef MAC
			if (inmenu) {
				if (comnum < 0)
					fprintf(of, "\t(data_obj *) NULL,\n");
				else
					fprintf(of, "\t(data_obj *) &%s[%d],\n",which, comnum);
			} else /*...*/
#endif
			{
				if (comnum < 0)
					fprintf(of, "\t(data_obj *) NULL,\t\t/* %s %s */\n", PPkey(ch), comname);
				else
					fprintf(of, "\t(data_obj *) &commands[%d],\t/* %s %s */\n", comnum, PPkey(ch), comname);
				ch += 1;
			}
		} else {
			/* If unrecognized, pass and prepare to start new table */
			fputs(line, of);
			ch = 0;
		}
	}
#ifdef MAC
}
#endif
	return 0;
}
