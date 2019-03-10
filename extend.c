/*************************************************************************
 * This program is copyright (C) 1985, 1986 by Jonathan Payne.  It is    *
 * provided to you without charge for use only on a licensed Unix        *
 * system.  You may copy JOVE provided that this notice is included with *
 * the copy.  You may not sell copies of this program or versions        *
 * modified for use on microcomputer systems, unless the copies are      *
 * included with a Unix system distribution and the source is provided.  *
 *************************************************************************/

#include "jove.h"
#include "io.h"
#include "termcap.h"
#include "ctype.h"
#ifdef JOB_CONTROL
#	include <signal.h>
#endif

#include <varargs.h>

int	InJoverc = 0;

extern int	getch(),
		getchar();

/* Auto execute code */

#define NEXECS	20

static struct {
	char	*a_pattern;
	data_obj	*a_func;
} AutoExecs[NEXECS] = {0};

static int	ExecIndex = 0;

/* Function auto-execute. */

FAutoExec()
{
	DefAutoExec(findcom);
}

/* Macro auto-execute. */

MAutoExec()
{
	DefAutoExec(findmac);
}

/* VARARGS0 */

DefAutoExec(func)
data_obj	*(*func)();
{
	data_obj	*d;
	char	*pattern;
	int	i;

	if (ExecIndex >= NEXECS)
		complain("Too many auto-executes, max %d.", NEXECS);
	if ((d = (*func)(ProcFmt, NOTHING)) == 0)
		return;
	pattern = ask((char *) 0, ": %f %s ", d->Name);
	for (i = 0; i < ExecIndex; i++)
		if ((AutoExecs[i].a_func == d) &&
		    (strcmp(pattern, AutoExecs[i].a_pattern) == 0))
		    	return;		/* Eliminate duplicates. */
	AutoExecs[ExecIndex].a_pattern = copystr(pattern);
	AutoExecs[ExecIndex].a_func = d;
	ExecIndex++;
}

/* DoAutoExec: NEW and OLD are file names, and if NEW and OLD aren't the
   same kind of file (i.e., match the same pattern) or OLD is 0 and it
   matches, we execute the function associated with that kind of file. */
 
DoAutoExec(new, old)
register char	*new,
		*old;
{
	register int	i;

	exp_p = 1;
	exp = 1;	/* So minor modes don't toggle.  We always want
			   them on. */
	if (new == 0)
		return;
	for (i = 0; i < ExecIndex; i++)
		if ((LookingAt(AutoExecs[i].a_pattern, new, 0)) &&
		    (old == 0 || !LookingAt(AutoExecs[i].a_pattern, old, 0)))
			ExecFunc(AutoExecs[i].a_func);
}

BindAKey()
{
	BindSomething(findcom);
}

BindMac()
{
	BindSomething(findmac);
}

extern int	EscPrefix(),
		CtlxPrefix(),
		MiscPrefix();

data_obj **
IsPrefix(fp)
data_obj	*fp;
{
	int	(*func)();

	if (fp == 0 || (fp->Type & TYPEMASK) != FUNCTION)
		return 0;
	func = ((struct funct *) fp)->f_func;
	if (func == EscPrefix)
		return pref1map;
	if (func == CtlxPrefix)
		return pref2map;
	if (func == MiscPrefix)
		return miscmap;
	return 0;
}

unbind_aux(c)
{
	if (c == CR || c == LF)
		return FALSE;	/* tells do_ask to return */
	Insert(c);
	return !FALSE;
}

UnbindC()
{
	char	*keys;
	data_obj	**map = mainmap;

	keys = do_ask("\r\n\01\02\03\04\05\06\010\011\013\014\016\017\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037", unbind_aux, (char *) 0, ProcFmt);
	if (keys == 0)
		return;
	for (;;) {
		if (keys[1] == '\0')
			break;
		if ((map = IsPrefix(map[*keys])) == 0)
			break;
		keys++;
	}
	if (keys[1] != 0)
		complain("That's not a legitimate key sequence.");
	map[keys[0]] = 0;
}
		
addgetc()
{
	int	c;

	if (!InJoverc)
		Asking = strlen(mesgbuf);
	c = getch();
	if (InJoverc) {
		if (c == '\n')
			return EOF;	/* this isn't part of the sequence */
		else if (c == '\\') {
			if ((c = getch()) == LF)
				complain("[Premature end of line]");
		} else if (c == '^') {
			if ((c = getch()) == '?')
				c = RUBOUT;
			else if (isalpha(c) || index("[\\]^_", c))
				c = c - '@';
			else
				complain("[Unknown control character]");
		}
	}

	Asking = 0;
	add_mess("%p ", c);

	return c;
}

BindWMap(map, lastkey, func)
data_obj	**map,
		*func;
{
	data_obj	**nextmap;
	int	c;

	c = addgetc();
	if (c == EOF) {
		if (lastkey == EOF)
			complain("[Empty key sequence]");
		complain("[Premature end of key sequence]");
	} else {
		if (nextmap = IsPrefix(map[c]))
			BindWMap(nextmap, c, func);
		else
			map[c] = func;
	}
}

/* VARARGS0 */

BindSomething(func)
data_obj	*(*func)();
{
	data_obj	*d;

	if ((d = (*func)(ProcFmt, NOTHING)) == 0)
		return;
	s_mess(": %f %s ", d->Name);
	BindWMap(mainmap, EOF, d);
}

/* Describe key */

DescWMap(map, key)
data_obj	**map;
{
	data_obj	*fp = map[key],
			**prefp;

	if (fp == 0)
		add_mess("is unbound.");
	else if (prefp = IsPrefix(fp))
		DescWMap(prefp, addgetc());
	else
		add_mess("is bound to %s.", fp->Name);
}

KeyDesc()
{
	s_mess(ProcFmt);
	DescWMap(mainmap, addgetc());
}

DescCom()
{
	data_obj	*dp;
	char	pattern[100],
		key_bind[40],
		*file = CMD_DB;
	File	*fp;

	if (!strcmp(LastFunc->Name, "describe-variable"))
		dp = (data_obj *) findvar(ProcFmt, NOTHING);
	else
		dp = (data_obj *) findcom(ProcFmt, NOTHING);

	if (dp == 0)
		return;
	fp = open_file(file, iobuff, F_READ, COMPLAIN, QUIET);
	Placur(ILI, 0);
	flusho();
	sprintf(pattern, "^\\.dc \"%s\" \"\\([^\"]*\\)\"", dp->Name);
	TOstart("Help", TRUE);
	for (;;) {
		if (f_gets(fp, genbuf) == EOF) {
			Typeout("There is no documentation for \"%s\".", dp->Name);
			goto outahere;
		}
		if ((strncmp(genbuf, ".dc", 3) == 0) && LookingAt(pattern, genbuf, 0))
			break;
	}
	/* found it ... let's print it */
	putmatch(1, key_bind, sizeof key_bind);
	if (strcmp("Not Bound", key_bind) == 0)
		Typeout("To invoke %s, type \"ESC X %s<cr>\".",
			dp->Name,
			dp->Name);
	else if (strcmp("(variable)", key_bind) == 0)
		Typeout(dp->Name);
	else
		Typeout("Type \"%s\" to invoke %s.", key_bind, dp->Name);
	Typeout("");
	while (f_gets(fp, genbuf) != EOF)
		if (strncmp(genbuf, ".dc", 3) == 0)
			goto outahere;
		else
			Typeout("%s", genbuf);
outahere:
	f_close(fp);
	TOstop();
}

DescBindings()
{
	extern int	Typeout();

	TOstart("Key Bindings", TRUE);
	DescMap(mainmap, NullStr);
	TOstop();
}

DescMap(map, pref)
data_obj	**map;
char	*pref;
{
	int	c1,
		c2 = 0,
		numbetween;
	char	keydescbuf[40];
	data_obj	**prefp;

	for (c1 = 0; c1 < 0200 && c2 < 0200; c1 = c2 + 1) {
		c2 = c1;
		if (map[c1] == 0)
			continue;
		while (++c2 < 0200 && map[c1] == map[c2])
			;
		c2--;
		numbetween = c2 - c1;
		if (numbetween == 1)
			sprintf(keydescbuf, "%s {%p,%p}", pref, c1, c2);
		else if (numbetween == 0)
			sprintf(keydescbuf, "%s %p", pref, c1);
		else
			sprintf(keydescbuf, "%s [%p-%p]", pref, c1, c2);
		if (prefp = IsPrefix(map[c1]))
			DescMap(prefp, keydescbuf);
		else
			Typeout("%-14s%s", keydescbuf, map[c1]->Name);
	}
}

static
find_binds(fp, buf)
struct funct	*fp;
char	*buf;
{
	char	*endp;

	buf[0] = '\0';
	fb_aux(fp, mainmap, (char *) 0, buf);
	endp = buf + strlen(buf) - 2;
	if ((endp > buf) && (strcmp(endp, ", ") == 0))
		*endp = '\0';
}

static
fb_aux(fp, map, prefix, buf)
register data_obj	*fp,
			**map;
char	*buf,
	*prefix;
{
	int	c1,
		c2;
	char	*bufp = buf + strlen(buf),
		prefbuf[20];
	data_obj	**prefp;

	for (c1 = c2 = 0; c1 < 0200 && c2 < 0200; c1 = c2 + 1) {
		c2 = c1;
		if (map[c1] == fp) {
			while (++c2 < 0200 && map[c1] == map[c2])
				;
			c2--;
			if (prefix)
				sprintf(bufp, "%s ", prefix);
			bufp += strlen(bufp);
			switch (c2 - c1) {
			case 0:
				sprintf(bufp, "%p, ", c1);
				break;
	
			case 1:
				sprintf(bufp, "{%p,%p}, ", c1, c2);
				break;
	
			default:
				sprintf(bufp, "[%p-%p], ", c1, c2);
				break;
			}
		}
		if (prefp = IsPrefix(map[c1])) {
			sprintf(prefbuf, "%p", c1);
			fb_aux(fp, prefp, prefbuf, bufp);
		}
		bufp += strlen(bufp);
	}
}

Apropos()
{
	register struct funct	*fp;
	register struct macro	*m;
	register struct variable	*v;
	char	*ans;
	int	anyfs = 0,
		anyvs = 0;
	char	buf[256];

	ans = ask((char *) 0, ": %f (keyword) ");
	TOstart("Help", TRUE);
	for (fp = commands; fp->Name != 0; fp++)
		if (sindex(ans, fp->Name)) {
			if (anyfs == 0) {
				Typeout("Commands");
				Typeout("--------");
			}
			find_binds(fp, buf);
			if (buf[0])
				Typeout(": %-30s(%s)", fp->Name, buf);
			else
				Typeout(": %s", fp->Name);
			anyfs++;
		}
	for (v = variables; v->Name != 0; v++)
		if (sindex(ans, v->Name)) {
			int	type = (v->v_flags & V_TYPEMASK);

			if (anyfs) {
				Typeout(NullStr);
				Typeout("Variables");
				Typeout("---------");
				anyfs = 0;
			}
			anyvs++;
			vpr_aux(v, buf);
			Typeout(": set %-26s%s", v->Name, buf);
		}
	for (m = macros; m != 0; m = m->m_nextm)
		if (sindex(ans, m->Name)) {
			if (anyvs || anyfs) {
				Typeout(NullStr);
				Typeout("Macros");
				Typeout("------");
				anyvs = 0;
			}
			find_binds((data_obj *) m, buf);
			if (buf[0])
				Typeout(": %-30s(%s)", m->Name, buf);
			else
				Typeout(": %-30s%s", "execute-macro", m->Name);
		}
	TOstop();
}

Extend()
{
	data_obj	*d;

	if (d = findcom(": ", NOTHING))
		ExecFunc(d);
}

#include "ctype.h"

/* Read a positive integer from CP.  It must be in base BASE, and
   complains if it isn't.  If allints is nonzero, all the characters
   in the string must be integers or we return -1; otherwise we stop
   reading at the first nondigit. */

chr_to_int(cp, base, allints)
register char	*cp;
{
	register int	c;
	int	value = 0;

	while (c = *cp++) {
		if (!isdigit(c)) {
			if (allints)
				return -1;
			break;
		}
		c = c - '0';
		if (c >= base)
			complain("You must specify in base %d.", base);
		value = value * base + c;
	}
	return value;
}

ask_int(prompt, base)
char	*prompt;
int	base;
{
	char	*val = ask((char *) 0, prompt);
	int	value = chr_to_int(val, base, 1);

	if (value < 0)
		complain("That's not a number!");
	return value;
}

static
vpr_aux(vp, buf)
register struct variable	*vp;
char	*buf;
{
	switch (vp->v_flags & V_TYPEMASK) {
	case V_BASE10:
		sprintf(buf, "%d", *(vp->v_value));
		break;

	case V_BASE8:
		sprintf(buf, "%o", *(vp->v_value));
		break;

	case V_BOOL:
		sprintf(buf, (*(vp->v_value)) ? "on" : "off");
		break;

	case V_STRING:
		sprintf(buf, "%s", (char *) vp->v_value);
		break;

	case V_CHAR:
		sprintf(buf, "%p", *(vp->v_value));
		break;
	}
}

PrVar()
{
	struct variable	*vp;
	char	prbuf[256];

	if ((vp = (struct variable *) findvar(ProcFmt, NOTHING)) == 0)
		return;
	vpr_aux(vp, prbuf);
	s_mess(": %f %s => %s", vp->Name, prbuf);
}

SetVar()
{
	struct variable	*vp;
	char	*prompt;

	if ((vp = (struct variable *) findvar(ProcFmt, NOTHING)) == 0)
		return;
	prompt = sprint(": %f %s ", vp->Name);

	switch (vp->v_flags & V_TYPEMASK) {
	case V_BASE10:
	case V_BASE8:
	    {
	    	int	value;

		value = ask_int(prompt, ((vp->v_flags & V_TYPEMASK) == V_BASE10)
					  ? 10 : 8);
		*(vp->v_value) = value;
	    	break;
	    }

	case V_BOOL:
	    {
	    	char	*def = *(vp->v_value) ? "off" : "on",
	    		*on_off;
	    	int	value;

	    	on_off = ask(def, prompt);
		if (casecmp(on_off, "on") == 0)
			value = ON;
	    	else if (casecmp(on_off, "off") == 0)
	    		value = OFF;
	    	else
	    		complain("Boolean variables must be ON or OFF.");
	    	*(vp->v_value) = value;
	    	s_mess("%s%s", prompt, value ? "on" : "off");
	    	break;
	    }

	case V_STRING:
	    {
		char	*str;

	    	/* Do_ask() so if you want you can set string to
		   "" if you so desire. */
	    	str = do_ask("\r\n", (int (*)()) 0, (char *) vp->v_value, prompt);
	    	if (str == 0)
			str = NullStr;
	    	strcpy((char *) vp->v_value, str);
		/* ... and hope there is enough room. */
	    	break;
	    }
	case V_CHAR:
		f_mess(prompt);
	    	*(vp->v_value) = addgetc();
		break;	    	

	}
	if (vp->v_flags & V_MODELINE)
		UpdModLine++;
	if (vp->v_flags & V_CLRSCREEN)
		ClAndRedraw();
	if (vp->v_flags & V_TTY_RESET)
		tty_reset();
}
			
/* Command completion - possible is an array of strings, prompt is
   the prompt to use, and flags are ... well read jove.h.

   If flags are RET_STATE, and the user hits <return> what they typed
   so far is in the Minibuf string. */

static char	**Possible;
static int	comp_value,
		comp_flags;

aux_complete(c)
{
	int	command,
		length,
		i;

	switch (c) {
	case EOF:
		comp_value = -1;
		return 0;

	case '\r':
	case '\n':
		command = match(Possible, linebuf);
		if (command >= 0) {
			comp_value = command;
			return 0;	/* tells ask to stop */
		}
		if (eolp() && bolp()) {
			comp_value = NULLSTRING;
			return 0;
		}
		if (comp_flags == RET_STATE) switch (command) {
			case UNIQUE:
			case ORIGINAL:
			case NULLSTRING:
				comp_value = command;
				return 0;

			default:
				break;
		}
		if (InJoverc)
			complain("[\"%s\" unknown]", linebuf);
		rbell();
		break;

	case '\t':
	case ' ':
	    {
		int	minmatch = 1000,
	    		maxmatch = 0,
	    		numfound = 0,
	    		lastmatch = -1,
			length = strlen(linebuf);

		for (i = 0; Possible[i] != 0; i++) {
			int	this_len;

			this_len = numcomp(Possible[i], linebuf);
			maxmatch = max(maxmatch, this_len);
			if (this_len >= length) {
				if (numfound)
					minmatch = min(minmatch, numcomp(Possible[lastmatch], Possible[i]));
				else
					minmatch = strlen(Possible[i]);
				numfound++;
				lastmatch = i;
				if (strcmp(linebuf, Possible[i]) == 0)
					break;
			}
		}

		if (numfound == 0) {
			rbell();
			if (InJoverc)
				complain("[\"%s\" unknown]", linebuf);
			/* If we're not in the .joverc then
			   let's do something helpful for the
			   user. */
			if (maxmatch < length) {
				char	*cp;

				cp = linebuf + maxmatch;
				*cp = 0;
				Eol();
			}
			break;
		}
	    	if (c != '\t' && numfound == 1) {
	    		comp_value = lastmatch;
			return 0;
		}
		null_ncpy(linebuf, Possible[lastmatch], minmatch);
	    	Eol();
		if (minmatch == length)	/* No difference */
			rbell();
		break;
	    }

	case '?':
		if (InJoverc)
			complain((char *) 0);
		/* kludge: in case we're using UseBuffers, in which case
		   linebuf gets written all over */
		strcpy(Minibuf, linebuf);
		length = strlen(Minibuf);
		TOstart("Completion", TRUE);	/* for now ... */
		for (i = 0; Possible[i]; i++)
			if (numcomp(Possible[i], Minibuf) >= length) {
				Typeout(Possible[i]);
				if (TOabort != 0)
					break;
			}

		TOstop();
		break;
	}
	return !FALSE;
}

complete(possible, prompt, flags)
register char	*possible[];
char	*prompt;
{
	if (!InJoverc) {
		Possible = possible;
		comp_flags = flags;
		(void) do_ask("\r\n \t?", aux_complete, NullStr, prompt);
		return comp_value;
	}
	/* This is for faster startup.  This just reads until a
	   space or a tab or a newline character is reached, and
	   then does lookup on that string.  This should be much
	   faster than initializing the minibuffer for each line. */
	{
		char	cmdbuf[128];
		register char	*cp = cmdbuf;
		register int	i,
				c;

		while (((c = getch()) != EOF) && !index(" \t\r\n", c))
			*cp++ = c;
		if (c == EOF)
			return EOF;
		*cp = '\0';
		if ((i = match(possible, cmdbuf)) >= 0 || i == NULLSTRING)
			return i;
		complain("[\"%s\" unknown]", linebuf);
		/* NOTREACHED */
	}
}

match(choices, what)
register char	**choices,
		*what;
{
	register int	len;
	int	i,
		found = 0,
		save,
		exactmatch = -1;

	len = strlen(what);
	if (len == 0)
		return NULLSTRING;
	for (i = 0; choices[i]; i++) {
		if (strncmp(what, choices[i], len) == 0) {
			if (strcmp(what, choices[i]) == 0)
				exactmatch = i;
			save = i;
			found++;	/* Found one. */
		}
	}

	if (found == 0)
		save = ORIGINAL;
	else if (found > 1) {
		if (exactmatch != -1)
			save = exactmatch;
		else
			save = AMBIGUOUS;
	}

	return save;
}

Source()
{
	char	*com,
		buf[FILESIZE];

	sprintf(buf, "%s/.joverc", getenv("HOME"));
	com = ask_file(buf, buf);
	if (joverc(buf) == NIL)
		complain(IOerr("read", com));
}

BufPos()
{
	register Line	*lp = curbuf->b_first;
	register int	i,
			dotline;

	for (i = 0; lp != 0; i++, lp = lp->l_next)
		if (lp == curline)
			dotline = i + 1;

	s_mess("\"%s\" line %d of %d --%d%%--, column %d of %d.",
			filename(curbuf),
			dotline,
			i,
			(int) (((long) dotline * 100) / i),
			1 + calc_pos(linebuf, curchar),
			1 + calc_pos(linebuf, strlen(linebuf)));
}

#define IF_UNBOUND	-1
#define IF_TRUE		1
#define IF_FALSE	!IF_TRUE

do_if(cmd)
char	*cmd;
{
	int	pid,
		status;

	switch (pid = fork()) {
	case -1:
		complain("[Fork failed: if]");

	case 0:
	    {
		char	*args[12],
			*cp = cmd,
			**ap = args;

	    	*ap++ = cmd;
	    	for (;;) {
			if ((cp = index(cp, ' ')) == 0)
				break;
			*cp++ = '\0';
			*ap++ = cp;
		}
		*ap = 0;

		close(0);	/*	we want reads to fail */
		/* close(1);	 but not writes or ioctl's
		close(2);    */

	    	(void) execvp(args[0], args);
		_exit(-10);	/* signals exec error (see below) */
	    }
	}
#ifdef IPROCS
	sighold(SIGCHLD);
#endif
	dowait(pid, &status);
#ifdef IPROCS
	sigrelse(SIGCHLD);
#endif
	if (status == -10)
		complain("[Exec failed]");
	if (status < 0)
		complain("[Exit %d]", status);
	return (status == 0);	/* 0 means successful */
}

joverc(file)
char	*file;
{
	char	buf[LBSIZE],
		lbuf[LBSIZE];
	int	lnum = 0,
		eof = FALSE;
	jmp_buf	savejmp;
	int	IfStatus = IF_UNBOUND;
	File	*fp;

	fp = open_file(file, buf, F_READ, !COMPLAIN, QUIET);
	if (fp == NIL)
		return NIL;

	/* Catch any errors, here, and do the right thing with them,
	   and then restore the error handle to whoever did a setjmp
	   last. */

	push_env(savejmp);
	if (setjmp(mainjmp)) {
		Buffer	*savebuf = curbuf;

		SetBuf(do_select((Window *) 0, "RC errors"));
		ins_str(sprint("%s:%d:%s\t%s\n", pr_name(file), lnum, lbuf, mesgbuf), NO);
		unmodify();
		SetBuf(savebuf);
		Asking = 0;
	}
	InJoverc = 1;
	if (!eof) do {
		eof = (f_gets(fp, lbuf) == EOF);
		lnum++;
		if (casencmp(lbuf, "if", 2) == 0) {
			char	cmd[128];

			if (IfStatus != IF_UNBOUND)
				complain("[Cannot have nested if's]");
			if (LookingAt("if[ \t]*\\(.*\\)$", lbuf, 0) == 0)
				complain("[If syntax error]");
			putmatch(1, cmd, sizeof cmd);
			IfStatus = do_if(cmd) ? IF_TRUE : IF_FALSE;
			continue;
		} else if (casencmp(lbuf, "else", 4) == 0) {
			if (IfStatus == IF_UNBOUND)
				complain("[Unexpected `else']");
			IfStatus = !IfStatus;
			continue;
		} else if (casencmp(lbuf, "endif", 5) == 0) {
			if (IfStatus == IF_UNBOUND)
				complain("[Unexpected `endif']");
			IfStatus = IF_UNBOUND;
			continue;
		}
		if (IfStatus == IF_FALSE)
			continue;
		(void) strcat(lbuf, "\n");
		Inputp = lbuf;
		while (*Inputp == ' ' || *Inputp == '\t')
			Inputp++;	/* skip white space */
		Extend();
	} while (!eof);

	f_close(fp);
	pop_env(savejmp);
	Inputp = 0;
	Asking = 0;
	InJoverc = 0;
	if (IfStatus != IF_UNBOUND)
		complain("[Missing endif]");
	return !NIL;
}
