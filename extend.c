/**************************************************************************
 * This program is Copyright (C) 1986-2002 by Jonathan Payne.  JOVE is    *
 * provided by Jonathan and Jovehacks without charge and without          *
 * warranty.  You may copy, modify, and/or distribute JOVE, provided that *
 * this notice is included in all the source files and documentation.     *
 **************************************************************************/

#include "jove.h"
#include "fp.h"
#include "jctype.h"
#include "chars.h"
#include "commands.h"
#include "disp.h"
#include "re.h"
#include "ask.h"
#include "extend.h"
#include "fmt.h"
#include "insert.h"
#include "move.h"
#include "sysprocs.h"
#include "proc.h"
/* #include "util.h" */
#include "vars.h"

#ifdef MAC
# include "mac.h"
#endif

private void
	DefAutoExec proto((const data_obj *(*proc) ptrproto((const char *))));

int	InJoverc = 0;

/* Auto execute code */

#define NEXECS	20

private struct AutoExec {
	const char	*a_pattern;
	const data_obj	*a_cmd;
	int	a_arg_state;
	long	a_arg_count;
} AutoExecs[NEXECS];	/* must be initialized by system to 0 */

private int	ExecIndex = 0;

/* Command auto-execute. */

void
CAutoExec()
{
	DefAutoExec(findcom);
}

/* Macro auto-execute. */

void
MAutoExec()
{
	DefAutoExec(findmac);
}

private void
DefAutoExec(proc)
const data_obj	*(*proc) ptrproto((const char *));
{
	const data_obj	*d = (*proc)(ProcFmt);
	const char	*pattern;
	register struct AutoExec	*p;

	pattern = do_ask("\r\n", NULL_ASK_EXT, (char *) NULL, ": %f %s ", d->Name);
	for (p = AutoExecs; p != &AutoExecs[ExecIndex]; p++)
		if (p->a_cmd == d
		&& ((pattern == NULL || p->a_pattern == NULL)?
			(pattern == p->a_pattern) : (strcmp(pattern, p->a_pattern) == 0))
		&& p->a_arg_state == arg_state
		&& p->a_arg_count == arg_count)
			return;		/* eliminate duplicates */

	if (ExecIndex >= NEXECS) {
		complain("Too many auto-executes, max %d.", NEXECS);
		/* NOTREACHED */
	}
	p->a_pattern = copystr(pattern);
	p->a_cmd = d;
	p->a_arg_state = arg_state;
	p->a_arg_count = arg_count;
	ExecIndex += 1;
}

/* DoAutoExec: NEW and OLD are file names, and if NEW and OLD aren't the
 * same kind of file (i.e., match the same pattern) or OLD is NULL and it
 * matches, OR if the pattern is NULL (none was specified) then, we execute
 * the command associated with that kind of file.
 */
void
DoAutoExec(new, old)
register char	*new,
		*old;
{
	register struct AutoExec	*p;

	for (p = AutoExecs; p != &AutoExecs[ExecIndex]; p++) {
		if (p->a_pattern == NULL
		|| ((new != NULL && LookingAt(p->a_pattern, new, 0))
		   && !(old != NULL && LookingAt(p->a_pattern, old, 0))))
		{
			arg_state = p->a_arg_state;
			arg_count = p->a_arg_count;
			ExecCmd(p->a_cmd);
			clr_arg_value();
		}
	}
}

ZXchar
addgetc()	/* NOTE: can return EOF */
{
	ZXchar	c;

	if (!InJoverc) {
		Asking = YES;
		AskingWidth = strlen(mesgbuf);
		c = getch();
		Asking = NO;
		add_mess("%p ", c);
	} else {
		c = getch();
		switch (c) {
		case '\n':
			c = EOF;	/* this isn't part of the sequence */
			break;
		case '\\':
		case '^':
			c = DecodePair(c, getch());
			break;
		}
	}
	return c;
}

void
Extend()
{
	ExecCmd(findcom(": "));
}

/* Read a positive long integer from CP.  It must be in base BASE, and
 * complains if it isn't.  If allints, all the characters
 * in the string must be integers or we return NO (failure); otherwise
 * we stop reading at the first nondigit and return YES (success).
 */
bool
chr_to_long(cp, base, allints, result)
register const char	*cp;
int	base;
bool	allints;
register long	*result;
{
	register char	c;
	long	value = 0;
	int	sign;

	if ((c = *cp) == '-') {
		sign = -1;
		cp += 1;
	} else
		sign = 1;
	while ((c = *cp++) != '\0') {
		if (!jisdigit(c)) {
			if (allints)
				return NO;

			break;
		}
		c = c - '0';
		if (c >= base) {
			complain("You must specify in base %d.", base);
			/* NOTREACHED */
		}
		value = value * base + c;
	}
	*result = value * sign;
	return YES;
}

/* Read a positive integer from CP.  It must be in base BASE, and
 * complains if it isn't.  If allints, all the characters
 * in the string must be integers or we return NO (failure); otherwise
 * we stop reading at the first nondigit and return YES (success).
 */
bool
chr_to_int(cp, base, allints, result)
register const char	*cp;
int	base;
bool	allints;
register int	*result;
{
	long	value;
	bool ret = chr_to_long(cp, base, allints, &value);
	if (ret == YES)
	    *result = (int)(value & ~0); /* XXX but no worse than before */
	return ret;
}

long
ask_long(def, prompt, base)
const char	*def;
const char	*prompt;
int	base;
{
	const char	*val = ask(def, prompt);
	long	value;

	if (!chr_to_long(val, base, YES, &value)) {
		complain("That's not a number!");
		/* NOTREACHED */
	}
	return value;
}

int
ask_int(def, prompt, base)
const char	*def;
const char	*prompt;
int	base;
{
	const char	*val = ask(def, prompt);
	int		value = 0;	/* avoid gcc complaint */

	if (!chr_to_int(val, base, YES, &value)) {
		complain("That's not a number!");
		/* NOTREACHED */
	}
	return value;
}

void
vpr_aux(vp, buf, size)
register const struct variable	*vp;
char	*buf;
size_t	size;
{
	switch (vp->v_flags & V_TYPEMASK) {
	case V_INT:
	case V_WHOLEX:
	case V_WHOLE:
	case V_NAT:
		swritef(buf, size, (vp->v_flags & V_FMODE)? "%03o" : "%d",
			*((int *) vp->v_value));
		break;

	case V_BOOL:
		swritef(buf, size, (*((int *) vp->v_value)) ? "on" : "off");
		break;

	case V_STRING:
	case V_FILENAME:
		swritef(buf, size, "%s", vp->v_value);
		break;

	case V_CHAR:
		swritef(buf, size, "%p", *((ZXchar *) vp->v_value));
		break;
	}
}

void
PrVar()
{
	struct variable	*vp = (struct variable *) findvar(ProcFmt);
	char	prbuf[MAXCOLS];

	vpr_aux(vp, prbuf, sizeof(prbuf));
	f_mess(": %f %s => %s", vp->Name, prbuf);
	stickymsg = YES;
}

void
InsVar()
{
	struct variable	*vp = (struct variable *) findvar(ProcFmt);
	char	prbuf[MAXCOLS];

	vpr_aux(vp, prbuf, sizeof(prbuf));
	ins_str(prbuf);
	stickymsg = YES;
}

void
vset_aux(vp, prompt)
const struct variable	*vp;
char	*prompt;
{
	if (vp->v_flags & V_READONLY)
		complain("[cannot set readonly variable %s]", vp->Name);
	switch (vp->v_flags & V_TYPEMASK) {
	case V_INT:
	case V_WHOLEX:
	case V_WHOLE:
	case V_NAT:
	    {
		char	def[30];
		static const int	lwbt[] = {
			~0,	/* V_INT -- I hope we are two's complement */
			-1,	/* V_WHOLEX */
			0,	/* V_WHOLE */
			1,	/* V_NAT */
			};
		int
			val,
			lwb = lwbt[(vp->v_flags & V_TYPEMASK) - V_INT];

		vpr_aux(vp, def, sizeof(def));
		val = ask_int(def, prompt, (vp->v_flags & V_FMODE)? 8 : 10);
		if (val < lwb) {
			complain("[%s must not be less than %d]", vp->Name, lwb);
			/* NOTREACHED */
		}
		*((int *) vp->v_value) = val;
		break;
	    }

	case V_BOOL:
	    {
		static const char	*possible[/*bool*/] = {"off", "on", NULL };
		bool	*valp = (bool *) vp->v_value;
		int	newval = complete(possible, possible[!*valp], prompt,
			CASEIND | ALLOW_OLD | ALLOW_EMPTY);

		if (newval < 0)
			newval = !*valp;
		*valp = newval;
#ifdef MAC
		MarkVar(vp, -1, 0);	/* mark the menu item */
#endif
		s_mess("%s%s", prompt, possible[newval]);
		break;
	    }

	case V_FILENAME:
	    {
		char	fbuf[FILESIZE];

		strcpy((char *) vp->v_value,
			ask_file(prompt, (char *) vp->v_value, fbuf));
		break;
	    }

	case V_STRING:
	    {
		/* do_ask() so you can set string to "" if you so desire */
		const char	*str = do_ask("\r\n", NULL_ASK_EXT,
			(char *) vp->v_value, prompt);

		jamstrsub((char *) vp->v_value, str == NULL? NullStr : str,
			vp->v_size);
		break;
	    }

	case V_CHAR:
		s_mess(prompt);
		*((ZXchar *) vp->v_value) = addgetc();
		break;
	}

	if (vp->v_flags & V_MODELINE)
		UpdModLine = YES;
	if (vp->v_flags & V_CLRSCREEN)
		ClAndRedraw();
	if (vp->v_flags & V_TTY_RESET)
		tty_adjust();
#ifdef UNIX
	if (vp->v_flags & V_UPDFREQ)
		SetClockAlarm(YES);
#endif
#if defined(USE_CTYPE) && !defined(NO_SETLOCALE)
	if (vp->v_flags & V_LOCALE) {
		locale_adjust();
		ClAndRedraw();
	}
#endif
}

void
SetVar()
{
	struct variable	*vp = (struct variable *) findvar(ProcFmt);
	char	prompt[128];

	swritef(prompt, sizeof(prompt), ": %f %s ", vp->Name);
	vset_aux(vp, prompt);
}

/* complete: buffer/command/keymap/macro/variable name completion.
 *
 * possible: an array of strings
 * prompt: the prompt to use
 * flags: a set of flags:
 *   CASEIND: ignore case
 *   ALLOW_OLD: allow answer listed in possible
 *   ALLOW_NEW: allow answer not listed in possible
 *   ALLOW_EMPTY: allow empty answer
 *
 * complete returns an index into possible, or -1 if there is no
 * right answer there, in which case what the user typed is in Minibuf.
 *
 * aux_complete is called by real_ask, on behalf of complete, to handle:
 *
 * ?        Typeout possible completions (does not change the answer)
 *
 * TAB      Extend answer as much as possible (this might actually
 *          involve shrinking the answer until it is the prefix of
 *          at least one Possible; if so, error).
 *
 * SP       If answer is complete (i.e. matches a whole entry in Possible),
 *          accept it.  Otherwise, extend answer as much as possible;
 *          if the result is unique, and we didn't shrink the answer,
 *          accept it.
 *
 * CR or NL If ALLOW_NEW and answer non-empty: accept the answer.
 *          If ALLOW_EMPTY and answer empty: accept the answer.
 *          If ALLOW_OLD and answer is complete: accept it.
 *          If ALLOW_OLD and answer is prefix of a unique Possible: accept that Possible.
 *          If ALLOW_INDEX and answer is a numeral that is a legitimate
 *          index into possible, accept as that possible.
 *          Otherwise: error
 *
 * If we are InJoverc, we cannot be interactive, so ? is forbidden
 * and all the others are treated as CR; an error is fatal.
 *
 * The following file-static variables smuggle values from complete
 * to aux_complete and vice versa, behind the back of do_ask etc.
 * This could be nicer if C supported nested procedures (closures);
 * perhaps we should simulate them.
 */

private const char	*const *Possible;	/* possible arg of complete */
private int
	comp_flags,	/* flags arg of complete */
	comp_value;	/* return value for complete; set by aux_complete */

private bool aux_complete proto((ZXchar c));	/* needed to comfort dumb MS Visual C */

private bool
aux_complete(c)
ZXchar	c;
{
	if (comp_flags & CASEIND) {
		char	lc;
		char	*lp;

		for (lp = linebuf; (lc = *lp) != '\0'; lp++)
			*lp = CharDowncase(lc);
	}
	if (c == '?') {
		int	i;
		size_t	len = strlen(linebuf);

		if (InJoverc) {
			complain("[invalid `?']");
			/* NOTREACHED */
		}
		/* kludge: in case we're using UseBuffers, in which case
		 * linebuf gets written all over (but restored by TOstop/TOabort)
		 */
		strcpy(Minibuf, linebuf);
		TOstart("Completion");	/* for now ... */
		for (i = 0; Possible[i] != NULL && !TOabort; i++) {
			if ((comp_flags & ALLOW_INDEX) && len == 0)
				Typeout("%2d %s", i+1, Possible[i]);
			else if (strncmp(Possible[i], Minibuf, len) == 0)
				Typeout("%s", Possible[i]);
		}

		TOstop();
	} else {
		/* let's do some completion! */
		int
			i,
			numeral = 0,		/* avoid gcc complaint */
			len = strlen(linebuf),
			minmatch = 1000,	/* init with dummy to placate picky compilers */
			maxmatch = 0,
			numfound = 0,
			lastmatch = -1;	/* init with dummy to placate picky compilers */

		if (InJoverc || c == '\n')
			c = '\r';	/* NL is synonym for CR; InJoverc, all are treated as CR */
		for (i = 0; Possible[i] != NULL; i++) {
			int	this_len = *Possible[i] == *linebuf
				? numcomp(Possible[i], linebuf) : 0;

			if (maxmatch < this_len)
				maxmatch = this_len;
			if (this_len >= len) {
				if (Possible[i][len] == '\0' && c != '\t') {
					/* an exact match */
					if (comp_flags & ALLOW_OLD) {
						comp_value = i;	/* good: done */
						return NO;
					} else {
						if (InJoverc) {
							complain("[%s already exists]");
							/* NOTREACHED */
						}
						add_mess(" [already exists]");
						SitFor(7);
						return YES;
					}
				}
				minmatch = numfound == 0
					? (int)strlen(Possible[i])
					: jmin(minmatch, numcomp(Possible[lastmatch], Possible[i]));
				numfound += 1;
				lastmatch = i;
			}
		}

		if (c == '\r' && len > 0
		&& (comp_flags & ALLOW_INDEX)
		&& chr_to_int(linebuf, 10, YES, &numeral)
		&& 0 < numeral && numeral <= i
		) {
			comp_value = numeral - 1;	/* accept as an index into possible */
			return NO;
		}

		if (c == '\r' && (comp_flags & (len == 0? ALLOW_EMPTY : ALLOW_NEW))) {
			comp_value = -1;	/* accept as new value (perhaps empty) */
			return NO;
		}
		if (numfound == 1 && c != '\t' && (comp_flags & ALLOW_OLD)) {
			comp_value = lastmatch;
			return NO;
		}
		if (InJoverc) {
			complain("[\"%s\" %s]", linebuf,
				numfound == 0? "unknown" : "ambiguous");
			/* NOTREACHED */
		}
		if (numfound == 0) {
			/* Unknown: either not ALLOW_NEW (bad) or not CR (not good enough) */
			add_mess(" [unknown]");
			SitFor(7);
			if (maxmatch < len && (comp_flags & ALLOW_NEW) == 0) {
				linebuf[maxmatch] = '\0';
				Eol();
			}
		} else {
			/* Ambiguous (or unique, but TAB): extend as much as
			 * possible without precluding any of the ambiguous matches.
			 * Explain if we were expected to be done (CR)
			 * or if we made no progress.
			 */
			null_ncpy(linebuf, Possible[lastmatch], (size_t) minmatch);
			modify();
			makedirty(curline);
			Eol();
			if (c == '\r' || minmatch == len) {
				add_mess(numfound == 1? " [complete]" : " [ambiguous]");
				SitFor(7);
			}
		}
	}
	return YES;
}

int
complete(possible, def, prompt, flags)
register const char	*const *possible;
const char	*def;
const char	*prompt;
int	flags;
{
	/* protect static "Possible" etc. from being overwritten due to recursion */
	if (InRealAsk) {
		complain((char *) NULL);
		/* NOTREACHED */
	}
	Possible = possible;
	comp_flags = flags;
	(void) do_ask("\r\n \t?", aux_complete, def, prompt);
	return comp_value;
}

void
Source()
{
	char
		fnamebuf[FILESIZE];
	bool	silence = is_an_arg();

	PathCat(fnamebuf, sizeof(fnamebuf), HomeDir,
#ifdef MSFILESYSTEM
		"jove.rc"
#else
		".joverc"
#endif
		);
	(void) ask_file((char *)NULL, fnamebuf, fnamebuf);
	if (!joverc(fnamebuf) && !silence) {
		message(IOerr("read", fnamebuf));
		complain((char *)NULL);
		/* NOTREACHED */
	}
}

/* TODO: Make this unsigned long when we dump support for pre-ANSI C */
/* calculate percentage without float and no overflow */
private int
calc_percent(a, b)
long	a, b;
{
	int v;
	if (b == 0) {
		v = 100;
	} else if (a > (~(1UL << ((sizeof(long)*CHAR_BIT)-1)))/100) {
		v = (int) (a / (b / 100));
	} else {
		v = (int) ((a * 100) / b);
	}
	return v;
}
	
/* TODO: Make dotchar, nchars unsigned long when we dump support for pre-ANSI C */
void
BufPos()
{
	register LinePtr	lp = curbuf->b_first;
	register long
		i,
		dotline = 0;	/* avoid uninitialized complaint from gcc -W */
	long	dotchar = 0;	/* avoid uninitialized complaint from gcc -W */
	long	nchars;

	for (i = nchars = 0; lp != NULL; i++, lp = lp->l_next) {
		if (lp == curline) {
			dotchar = nchars + curchar;
			dotline = i + 1;
		}
		nchars += length(lp) + (lp->l_next != NULL);	/* include the NL */
	}

	f_mess("[\"%s\" line %D/%D, char %D/%D (%d%%), cursor = %d/%d]",
	       filename(curbuf), dotline, i, dotchar, nchars,
	       calc_percent(dotchar, nchars),
	       calc_pos(linebuf, curchar),
	       calc_pos(linebuf, (int)strlen(linebuf)));
	stickymsg = YES;
}

#ifdef SUBSHELL

private bool
do_if(cmd)
char	*cmd;
{
	char	*args[12];

	/* Parse cmd into an argv.  Handle various quotes
	 * but not environment variable references.
	 */
	{
		char
			*ip = cmd,
			*op = cmd,
			**ap = args;

		for (;;) {
			while (jiswhite(*ip))
			    ip++;
			if (*ip == '\0')
				break;

			if (ap == &args[elemsof(args)]) {
				complain("Too many args for IF shell command");
				/* NOTREACHED */
			}
			*ap++ = op;
			for (;;) {
				char
					c = *ip++,
					c2;

				switch (c) {
				case '\0':
					ip -= 1;
					/*FALLTHROUGH*/
				case ' ':
				case '\t':
					break;
				case '"':
				case '\'':
					while ((c2 = *ip++) != c) {
						switch (c2) {
						case '\0':
							complain("Unpaired quote in IF command");
							/*NOTREACHED*/
						case '\\':
							if (c == '"') {
								c2 = *ip++;
								if (c2 == '\0') {
									complain("Misplaced \\ in IF command");
									/* NOTREACHED */
								}
							}
							/*FALLTHROUGH*/
						default:
							*op++ = c2;
							break;
						}
					}
					continue;
				case '\\':
					c = *ip++;
					if (c == '\0') {
						complain("Misplaced \\ in IF command");
						/* NOTREACHED */
					}
					/*FALLTHROUGH*/
				default:
					*op++ = c;
					continue;
				}
				break;
			}
			*op++ = '\0';
		}
		*ap = NULL;
	}

	/* Exec the parsed command */
# ifdef UNIX
	{
		wait_status_t status;

		switch (ChildPid = fork()) {
		case -1:
			complain("[Fork failed for IF: %s]", strerror(errno));
			/*NOTREACHED*/

		case 0:
			close(0);	/*	we want reads to fail */
			/* close(1);	 but not writes or ioctl's */
			/* close(2); */
			(void) execvp(args[0], args);
			_exit(errno);
			/*NOTREACHED*/
		}
		dowait(&status);
		if (!WIFEXITED(status)) {
			complain("[no status returned from child in IF test]");
			/* NOTREACHED */
		}
		if (WIFSIGNALED(status)) {
			complain("[IF test terminated by signal %d]", WTERMSIG(status));
			/* NOTREACHED */
		}
		return WEXITSTATUS(status)==0;
	}
# else
#  ifdef MSDOS_PROCS
	{
		int	status;

		if ((status = spawnvp(0, args[0], args)) < 0) {
			complain("[Spawn failed: IF]");
			/* NOTREACHED */
		}
		return (status == 0);	/* 0 means successful */
	}
#  else /* !MSDOS_PROCS */
	I do not know how to do this
#  endif /* !MSDOS_PROCS */
# endif /* !UNIX */
}
#endif /* SUBSHELL */

private bool
cmdmatch(inp, verb, oppat)
char	*inp;
char	*verb;
char	*oppat;
{
	int	len = strlen(verb);

	if (caseeqn(inp, verb, (size_t)len) && LookingAt("[ \\t]\\|$", inp, len)) {
		if (!LookingAt(oppat, inp, len)) {
			complain("[malformed %s]", verb);
			/* NOTREACHED */
		}
		return YES;
	}
	return NO;
}

bool
joverc(file)
char	*file;
{
	char	buf[LBSIZE],
		lbuf[LBSIZE];

	jmp_buf	savejmp;
	volatile int	lnum = 0;
	File	*volatile fp;
	volatile bool	eof = NO;
	volatile unsigned int	/* bitstrings */
			finger = 1,
			skipping = 0,
			inelse = 0;

	fp = open_file(file, buf, F_READ, NO);
	if (fp == NULL)
		return NO;

	/* Catch any errors, here, and do the right thing with them,
	 * and then restore the error handle to whoever did a setjmp
	 * last.
	 */
	InJoverc += 1;
	push_env(savejmp);
	if (setjmp(mainjmp)) {
		Buffer	*savebuf = curbuf;

		SetBuf(do_select((Window *)NULL, "RC errors"));
		ins_str(sprint("%s:%d:%s", pr_name(file, YES), lnum, lbuf));
		ins_str(sprint("\t%s\n", mesgbuf));
		unmodify();
		SetBuf(savebuf);
		Asking = NO;
	}
	while (!eof) {
		/* This peculiar delayed EOF testing allows the last line to
		 * end without a NL.  We add NL later, so we leave room for it.
		 */
		eof = f_gets(fp, lbuf, sizeof(lbuf)-1);
		lnum += 1;
		Inputp = lbuf;
		while (jiswhite(*Inputp))
			Inputp += 1;	/* skip white space */
		if (*Inputp == '#' || *Inputp == '\0') {
			/* a comment */
#ifdef SUBSHELL
		} else if (cmdmatch(Inputp, "if", "[ \t]*\\(.*\\)$")) {
			char	cmd[128];

			finger <<= 1;
			if (finger == 0) {
				complain("[`if' nested too deeply]");
				/* NOTREACHED */
			}
			putmatch(1, cmd, sizeof cmd);
			jdbg("if finger=0x%x skipping=0x%x inelse=0x%x cmd=\"%s\"\n", finger, skipping, inelse, cmd);
			if (skipping == 0 && !do_if(cmd))
				skipping |= finger;
#endif /* SUBSHELL */
#ifndef MAC	/* no environment in MacOS */
		} else if (cmdmatch(Inputp, "ifenv", "\\>[ \t]*\\<\\([^ \t][^ \t]*\\)\\>[ \t]\\(.*\\)$")) {
			finger <<= 1;
			if (finger == 0) {
				complain("[`ifenv' nested too deeply]");
				/* NOTREACHED */
			}
			jdbg("ifenv finger=0x%x skipping=0x%x inelse=0x%x\n", finger, skipping, inelse);
			if (skipping == 0) {
				char	envname[128],
					envpat[128],
					*envval;

				putmatch(1, envname, sizeof envname);
				putmatch(2, envpat, sizeof envpat);
				envval = getenv(envname);
				if (envval==NULL || !LookingAt(envpat, envval, 0))
					skipping |= finger;
			}
#endif
		} else if (cmdmatch(Inputp, "ifvar", "\\>[ \t]*\\<\\([^ \t][^ \t]*\\)\\>[ \t]\\(.*\\)$")) {
			finger <<= 1;
			if (finger == 0) {
				complain("[`if' nested too deeply]");
				/* NOTREACHED */
			}
			jdbg("ifvar finger=0x%x skipping=0x%x inelse=0x%x\n", finger, skipping, inelse);
			if (skipping == 0) {
				char	vname[128],
					vpat[128],
					vbuf[128],
					*val;
				bool	matched = NO;

				putmatch(1, vname, sizeof vname);
				putmatch(2, vpat, sizeof vpat);
				val = (char *)getvar(vname, vbuf, sizeof(vbuf));
				jdbg("ifvar var \"%s\" => \"%s\" vpat \"%s\"\n", vname, val != NULL? val : "(null)", vpat);
				if (val==NULL || !(matched = LookingAt(vpat, val, 0)))
					skipping |= finger;
				jdbg("ifvar matched=%d skipping=0x%x\n", matched, skipping);
			}
		} else if (cmdmatch(Inputp, "else", "[ \\t]*$")) {
			if (finger == 1 || (inelse & finger)) {
				complain("[Unexpected `else']");
				/* NOTREACHED */
			}
			inelse |= finger;
			skipping ^= finger;
			jdbg("else finger=0x%x skipping=0x%x inelse 0x%x\n", finger, skipping, inelse);
		} else if (cmdmatch(Inputp, "endif", "[ \\t]*$")) {
			if (finger == 1) {
				complain("[Unexpected `endif']");
				/* NOTREACHED */
			}
			inelse &= ~finger;
			skipping &= ~finger;
			finger >>= 1;
			jdbg("endif finger=0x%x skipping=0x%x inlse=0x%x\n", finger, skipping, inelse);
		} else if (skipping == 0) {
			(void) strcat(Inputp, "\n");
			for (;;) {
				ZXchar	c;

				cmd_sync();
				this_cmd = OTHER_CMD;

				if ((c = ZXC(*Inputp)) == '-' || jisdigit(c)) {
					LastKeyStruck = c;
					Inputp += 1;
					Digit();
				} else {
					Extend();
				}

				/* get any pending input hiding in the peek buffer */

				if ((c = peekchar) != EOF) {
					peekchar = EOF;
					if (Inputp != NULL && ZXRC(Inputp[-1]) == c) {
						Inputp -= 1;
					} else if (!jiswhite(c) && c != '\n' && c != '\0') {
						complain("[junk at end of line]");
						/* NOTREACHED */
					}
				}

				if (Inputp == NULL)
					break;

				while (jiswhite(*Inputp))
					Inputp += 1;	/* skip white space */
				if (*Inputp == '\0' || *Inputp == '\n')
					break;

				if (this_cmd != ARG_CMD) {
					complain("[junk at end of line]");
					/* NOTREACHED */
				}
			}
		}
	}

#ifdef SUBSHELL
	if (finger != 1) {
		finger = 1;
		complain("[Missing endif]");
		/* NOTREACHED */
	}
#endif
	f_close(fp);
	pop_env(savejmp);
	Inputp = NULL;
	Asking = NO;
	InJoverc -= 1;
	return YES;
}
