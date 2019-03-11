/***************************************************************************
 * This program is Copyright (C) 1986, 1987, 1988 by Jonathan Payne.  JOVE *
 * is provided to you without charge, and with no warranty.  You may give  *
 * away copies of JOVE, including sources, provided that this notice is    *
 * included in all the files.                                              *
 ***************************************************************************/

#include <stdio.h>
#include "jove.h"
#include "commands.h"
#include "vars.h"

#define LINESIZE	100	/* hope this is big enough */
#define STACKLIMIT	10	/* max conditional depth */

#define	PROC(p)	NULL	/* discard function pointers */
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

#ifdef	MAC
matchvar(choices, what)
register struct variable choices[];
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

static int
StartsWith(s, pre)
const char *s, *pre;
{
    return strncmp(s, pre, strlen(pre)) == 0;
}

private char *
PPchar(c)
int	c;
{
	static char	str[16];
	char	*cp = str;

	if (c & 0200) {
		c &= ~0200;
		strcpy(cp, "M-");
		cp += 2;
	}
	if (c == '\033')
		strcpy(cp, "ESC");
	else if (c < ' ')
		(void) sprintf(cp, "C-%c", c + '@');
	else if (c == '\177')
		strcpy(cp, "^?");
	else
		(void) sprintf(cp, "%c", c);
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
#ifdef	MAC
_main()		/* for Mac, so we can use redirection */
#else
main()
#endif
{
	FILE
		*ifile,
		*of;
	char
		line[LINESIZE],
		comname[LINESIZE];
	int
		comnum,
		lino = 0,
		ch = 0;
	struct {
		int	first;
		int	last;
		char	condition[LINESIZE];
	}
		stackspace[STACKLIMIT],	/* first entry not used */
		*sp = stackspace;
#ifdef	MAC
	char	*which;
	bool	inmenu = NO;
#endif

	ifile = stdin;
	of = stdout;
	if (ifile == NULL || of == NULL) {
		fprintf(stderr, "Cannot read input or write output.\n");
		exit(1);
	}
	for (comnum = 1; commands[comnum].Name != NULL; comnum++) {
		if (strcmp(commands[comnum-1].Name, commands[comnum].Name) >= 0) {
			fprintf(stderr, "command %s is out of order\n",
				commands[comnum].Name);
			exit(1);
		}
	}
	for (comnum = 1; variables[comnum].Name != NULL; comnum++) {
		if (strcmp(variables[comnum-1].Name, variables[comnum].Name) >= 0) {
			fprintf(stderr, "variable %s is out of order\n", variables[comnum].Name);
			exit(1);
		}
	}
	while (fgets(line, sizeof line, ifile) != NULL) {
		lino += 1;
		if (StartsWith(line, "#if")) {
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
			fprintf(of, line);
		} else if (StartsWith(line, "#else")) {
			if (sp == stackspace || sp->last != -1) {
				fprintf(stderr, "ifdef/endif mismatch at line %d!\n",
					lino);
				exit(1);
			}
			sp->last = ch;
			ch = sp->first;
			fprintf(of, line);
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
			fprintf(of, line);
#ifdef	MAC
		} else if (StartsWith(line, "#MENU")) {
			inmenu = YES;
#endif
		} else if (StartsWith(line, "\t\"")) {
			extract(comname, line);
			if (strcmp(comname, "unbound") == 0) {
				comnum = -1;
			} else {
				comnum = matchcmd(commands, comname);
#ifdef	MAC
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
#ifdef	MAC
			if (inmenu) {
				if (comnum < 0)
					fprintf(of, "\t(data_obj *) NULL,\n");
				else
					fprintf(of, "\t(data_obj *) &%s[%d],\n",which, comnum);
			} else /*...*/
#endif
			{
				if (comnum < 0)
					fprintf(of, "\t(data_obj *) NULL,\t\t/* %s */\n", PPchar(ch));
				else
					fprintf(of, "\t(data_obj *) &commands[%d],\t/* %s */\n", comnum, PPchar(ch));
				ch += 1;
			}
		} else {
			/* If unrecognized, pass and prepare to start new table */
			fprintf(of, line);
			ch = 0;
		}
	}
	if (sp != stackspace) {
		fprintf(stderr, "EOF inside #if\n");
		exit(1);
	}
	fclose(of);
	fclose(ifile);
	return 0;
}
